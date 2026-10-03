/*
** Collett - Project Class
** =======================
**
** This file is a part of Collett
** Copyright (C) 2025 Veronica Berglyd Olsen
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "counting.h"
#include "project.h"
#include "settings.h"
#include "storage.h"
#include "tools.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCursor>
#include <QTextDocumentFragment>
#include <QUrl>

#include <utility>

using namespace Qt::Literals::StringLiterals;

namespace Collett {

// Constructor/Destructor
// ======================

Project::Project()
{
    m_autoSaveTimer = new QTimer(this);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &Project::onAutoSave);

    m_countTimer = new QTimer(this);
    m_countTimer->setSingleShot(true);
    m_countTimer->setInterval(500);
    connect(m_countTimer, &QTimer::timeout, this, &Project::countDocuments);

    m_spell = new SpellChecker(this);

    // A project without its own spelling language follows the global one
    Settings *settings = Settings::instance();
    connect(settings, &Settings::spellLanguageChanged, this, [this]() {
        if (m_data && !m_data->hasSpellLanguage()) m_spell->setLanguage(Settings::instance()->spellLanguage());
    });
    connect(settings, &Settings::editorAutoSaveChanged, this, [this]() {
        m_autoSaveTimer->setInterval(Settings::instance()->editorAutoSave() * 1000);
    });
}

Project::~Project()
{
    qDebug() << "Destructor: Project";
}

// Public Methods
// ==============

bool Project::openProject(const QString &path)
{

    m_store = new Storage(path, false, this);
    qInfo() << "Loading Project:" << m_store->projectPath();
    if (!m_store->isValid()) {
        qWarning() << "Cannot load project from this path";
        return false;
    }

    m_data = new ProjectData(this);
    m_tree = new Tree(this);

    if (m_store->isNewProject()) {
        // Brand new project, nothing to read yet. Write out the initial
        // project files so they exist on disk right away.
        m_isValid = true;
        m_autoSaveTimer->setInterval(Settings::instance()->editorAutoSave() * 1000);
        m_autoSaveTimer->start();
        this->setupSpelling();
        emit projectChanged();
        return this->saveProject();
    }

    QJsonObject jData, jTree;

    if (!m_store->readProject(jData)) {
        m_lastError = m_store->lastError();
        return false;
    }
    m_data->unpack(jData);

    if (!m_store->readStructure(jTree)) {
        m_lastError = m_store->lastError();
        return false;
    }
    m_tree->unpack(jTree);
    for (Group *group : m_tree->groups()) {
        if (!this->loadGroup(group)) {
            m_lastError = m_store->lastError();
            return false;
        }
    }
    m_tree->showNovelGroup();

    m_isValid = true;
    m_autoSaveTimer->setInterval(Settings::instance()->editorAutoSave() * 1000);
    m_autoSaveTimer->start();
    this->setupSpelling();
    emit projectChanged();

    return true;
}

/**! @brief Open an existing project from QML, closing the open one first.
 *
 * The location is the project file, as a path or a file URL. Returns an
 * error message, or an empty string if the project was opened.
 */
QString Project::openProjectAt(const QString &location)
{
    const QUrl url(location);
    const QFileInfo info(url.isLocalFile() ? url.toLocalFile() : location);
    if (!info.isFile()) {
        return tr("There is no project file at this location.");
    }
    if (!this->closeProject()) {
        return tr("The open project could not be saved: %1").arg(m_lastError);
    }
    if (!this->openProject(info.absoluteFilePath())) {
        const QString error = m_lastError.isEmpty() ? tr("Could not open the project.") : m_lastError;
        this->releaseProject();
        return error;
    }
    return QString();
}

/**!
 * @brief Create a new project in a folder named after it.
 *
 * The project folder is made in the location, and the project starts with a
 * single page with the project name as a centred title. A project cannot be
 * created while another is open.
 *
 * @param location The folder to create the project folder in, as a path or
 *                 a local file URL.
 * @param name     The name of the project.
 * @return QString An empty string if the project was created, or otherwise
 *                 a message saying why not.
 */
QString Project::createProject(const QString &location, const QString &name)
{
    if (m_isValid) {
        return tr("A project is already open.");
    }

    const QString title = name.simplified();
    if (title.isEmpty()) {
        return tr("The project needs a name.");
    }

    const QUrl url(location);
    const QDir parent(url.isLocalFile() ? url.toLocalFile() : location);
    if (location.isEmpty() || !parent.exists()) {
        return tr("The location does not exist.");
    }

    // The folder is named after the project, without characters that are
    // not allowed in folder names on all platforms
    QString folder = title;
    for (const QChar c : QStringLiteral("/\\:*?\"<>|")) {
        folder.replace(c, u'_');
    }
    const QDir dir(parent.filePath(folder));
    if (dir.exists() && !dir.isEmpty()) {
        return tr("There is already a folder named \"%1\" in this location.").arg(folder);
    }
    if (!parent.mkpath(folder)) {
        return tr("Could not create the folder \"%1\".").arg(folder);
    }

    if (!this->openProject(dir.filePath("CollettProject.collett"))) {
        return m_lastError.isEmpty() ? tr("Could not create the project.") : m_lastError;
    }
    m_data->setName(title);

    // The title page, in the same format as the group files
    Node *node = m_tree->createNode(ItemLevel::PageLevel);
    Document *doc = new Document(node->handle(), this);
    QJsonObject heading;
    heading["u:fmt"_L1] = "h1:ac";
    heading["u:txt"_L1] = "t|" + title;
    QJsonObject data;
    data["m:created"_L1] = QDateTime::currentDateTime().toString(Qt::ISODate);
    data["x:content"_L1] = QJsonArray({heading});
    doc->unpack(data);
    doc->setModified(true);

    m_documents.insert(node->handle(), doc);
    trackDocument(doc);
    queueCount(node->handle());
    this->model()->insertNode(0, node);
    m_data->setLastEditedHandle(node->handle());

    this->saveProject();
    emit projectChanged();
    return QString();
}

bool Project::saveProject()
{

    if (m_store == nullptr || m_data == nullptr) {
        qWarning() << "Project storage not initialised, cannot save";
        return false;
    }

    countDocuments();
    qInfo() << "Saving Project:" << m_store->projectPath();
    if (!m_store->isValid()) {
        qWarning() << "Project storage invalid, cannot save";
        return false;
    }

    // Only the parts that have changed since they were read or last saved
    // are written
    if (m_data->isModified()) {
        QJsonObject jData;
        m_data->pack(jData);
        if (!m_store->writeProject(jData)) {
            m_lastError = m_store->lastError();
            return false;
        }
        m_data->setModified(false);
    }

    if (m_tree->isModified()) {
        QJsonObject jTree;
        m_tree->pack(jTree);
        if (!m_store->writeStructure(jTree)) {
            m_lastError = m_store->lastError();
            return false;
        }
        m_tree->setModified(false);
    }

    bool result = true;
    for (Group *group : m_tree->groups()) {
        result &= this->saveGroup(group);
    }
    return result;
}

/**! @brief Save and close the project.
 *
 * Nothing is closed if the project could not be saved. Returns true if no
 * project is open afterwards.
 */
bool Project::closeProject()
{
    if (m_isValid && !this->saveProject()) {
        return false;
    }
    this->releaseProject();
    return true;
}

/**! @brief Save the project to a new location.
 *
 * Everything is marked as modified first, so it is all written to the new
 * location, not just what has changed.
 */
bool Project::saveProjectAs(const QString &path)
{
    if (m_store) delete m_store;
    m_store = new Storage(path, false, this);
    m_isValid = true;
    if (m_data) m_data->setModified(true);
    if (m_tree) {
        m_tree->setModified(true);
        for (Group *group : m_tree->groups()) {
            group->setModified(true);
        }
    }
    return this->saveProject();
}

// Property Setters
// ================

/**! @brief Record the document last edited, so it is reopened on launch.
 *
 * The editor calls this when a scene gets focus. The project is not
 * notified of the change, as the value is only read when it is opened.
 */
void Project::setLastEditedHandle(const QString &handle)
{
    if (m_data) m_data->setLastEditedHandle(handle);
}

// Document Methods
// ================

/**!
 * @brief Get a document by its handle, creating it if needed.
 *
 * The documents of a group are read along with the group, so a document is
 * only created here for a node that has no text yet.
 *
 * @param handle     The handle of the document to open.
 * @return Document* The document, or nullptr if the project has no storage.
 */
Document *Project::openDocument(const QString &handle)
{
    if (!m_store) {
        return nullptr;
    }

    if (m_documents.contains(handle)) {
        return m_documents.value(handle);
    }

    Document *doc = new Document(handle, this);
    m_documents.insert(handle, doc);
    trackDocument(doc);
    queueCount(handle);

    return doc;
}

/**!
 * @brief Move the text after a cursor position into a new scene.
 *
 * Empty paragraphs left at the split are dropped. The undo history of both
 * documents is cleared, as undoing in only one would lose or duplicate text.
 *
 * @param handle    The handle of the document to split.
 * @param position  The cursor position to split at.
 * @return QString  The handle of the new document, or an empty string.
 */
QString Project::splitDocument(const QString &handle, int position)
{
    Document *doc = m_documents.value(handle, nullptr);
    ProjectModel *projectModel = this->model();
    const int row = projectModel ? projectModel->rowOf(handle) : -1;
    if (!doc || row < 0 || position < 0 || position >= doc->characterCount()) {
        return QString();
    }

    QTextCursor split(doc);
    split.setPosition(position);
    split.insertBlock();
    const QTextBlock block = split.block();

    // Copy the text to move. The block format of the first paragraph is kept
    // separately, as inserting a fragment into an empty document does not
    // carry it over.
    QTextDocumentFragment moved;
    QTextBlockFormat firstFormat;
    const QTextBlock first = block.length() == 1 ? block.next() : block;
    if (first.isValid()) {
        QTextCursor selection(doc);
        selection.setPosition(first.position());
        selection.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        moved = selection.selection();
        firstFormat = first.blockFormat();
    }

    // Remove the paragraph break before the paragraph, and everything after
    QTextCursor cut(doc);
    cut.setPosition(block.position() - 1);
    cut.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cut.removeSelectedText();

    // Remove an empty paragraph left at the end
    const QTextBlock last = doc->lastBlock();
    if (last.length() == 1 && last.previous().isValid()) {
        cut.setPosition(last.position() - 1);
        cut.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cut.removeSelectedText();
    }

    Node *node = m_tree->createNode(ItemLevel::SceneLevel);
    Document *newDoc = new Document(node->handle(), this);
    if (!moved.isEmpty()) {
        QTextCursor insert(newDoc);
        insert.insertFragment(moved);
        insert.movePosition(QTextCursor::Start);
        insert.setBlockFormat(firstFormat);
    }

    for (Document *changed : {doc, newDoc}) {
        changed->setUndoRedoEnabled(false);
        changed->setUndoRedoEnabled(true);
        changed->setModified(true);
    }

    m_documents.insert(node->handle(), newDoc);
    trackDocument(newDoc);
    queueCount(node->handle());
    projectModel->insertNode(row + 1, node);
    return node->handle();
}

/**!
 * @brief Delete a document from the project.
 *
 * The last document in the group cannot be deleted. Editors showing the
 * document are told before it is deleted, so they can let go of it.
 *
 * @param handle The handle of the document to delete.
 * @return bool  True if the document was deleted.
 */
bool Project::deleteDocument(const QString &handle)
{
    ProjectModel *projectModel = this->model();
    const int row = projectModel ? projectModel->rowOf(handle) : -1;
    if (row < 0 || projectModel->rowCount() <= 1) {
        return false;
    }

    delete projectModel->takeNode(row);
    m_tree->forgetNode(handle);
    if (Document *doc = m_documents.take(handle)) {
        emit documentDeleting(doc);
        doc->deleteLater();
    }
    return true;
}

/**!
 * @brief Merge a document into the document before it.
 *
 * The text is added after the text of the previous document as new
 * paragraphs, with its formatting, and the document is deleted. The previous
 * document keeps its title, type and other settings. As with a split, the
 * undo history of the previous document is cleared.
 *
 * @param handle The handle of the document to merge.
 * @return int   The position in the previous document where the merged
 *               text starts, or -1 if the document could not be merged.
 */
int Project::mergeDocument(const QString &handle)
{
    ProjectModel *projectModel = this->model();
    const int row = projectModel ? projectModel->rowOf(handle) : -1;
    if (row <= 0) {
        return -1;
    }

    const QString intoHandle = projectModel->data(projectModel->index(row - 1), ProjectModel::HandleRole).toString();
    Document *into = this->openDocument(intoHandle);
    Document *from = this->openDocument(handle);
    if (!into || !from) {
        return -1;
    }

    QTextCursor cursor(into);
    cursor.movePosition(QTextCursor::End);
    int position = cursor.position();
    if (!from->isEmpty()) {
        QTextCursor all(from);
        all.select(QTextCursor::Document);
        const QTextDocumentFragment moved = all.selection();
        const QTextBlockFormat firstFormat = from->firstBlock().blockFormat();

        if (!into->isEmpty()) {
            cursor.insertBlock();
            position = cursor.position();
        }
        cursor.insertFragment(moved);
        cursor.setPosition(position);
        cursor.setBlockFormat(firstFormat);

        into->setUndoRedoEnabled(false);
        into->setUndoRedoEnabled(true);
        into->setModified(true);
    }

    this->deleteDocument(handle);
    return position;
}

// Private Slots
// =============

/**! @brief Save the project structure and the open documents.
 *
 * The structure holds the document titles, so it is saved along with the
 * documents.
 */
void Project::onAutoSave()
{
    this->saveProject();
}

/**! @brief Count the documents that changed since the last count, and
 * update their counts in the project structure.
 */
void Project::countDocuments()
{
    m_countTimer->stop();
    ProjectModel *projectModel = this->model();
    if (!projectModel) {
        m_countQueue.clear();
        return;
    }
    for (const QString &handle : std::as_const(m_countQueue)) {
        if (const Document *doc = m_documents.value(handle, nullptr)) {
            projectModel->setCounts(projectModel->rowOf(handle), TextCounter::standardCount(TextCounter::snapshot(doc)));
        }
    }
    m_countQueue.clear();
}

// Private Methods
// ===============

/**! @brief Read the documents of a group, with their text, from the
 * group's content file.
 *
 * Entries that are not valid nodes are skipped, as are those with a handle
 * that is already in use. Returns false if the file could not be read.
 */
bool Project::loadGroup(Group *group)
{
    QJsonObject jGroup;
    if (!m_store->readContent(group->contentName(), jGroup)) {
        return false;
    }
    group->setCreatedTime(JsonUtils::unpackCreated(jGroup, group->createdTime()));

    for (const QJsonValue &value : jGroup["x:items"_L1].toArray()) {
        const QJsonObject item = value.toObject();
        Node *node = Node::unpack(item);
        if (!node) continue;
        if (!m_tree->addNode(group, node)) {
            delete node;
            continue;
        }
        Document *doc = new Document(node->handle(), this);
        doc->unpack(item);
        m_documents.insert(node->handle(), doc);
        trackDocument(doc);
    }
    group->setModified(false);
    return true;
}

/**! @brief Write a group's documents, with their text, to the group's
 * content file.
 *
 * The file is only written if the group or the text of one of its
 * documents has changed since it was read or last saved.
 */
bool Project::saveGroup(Group *group)
{
    bool modified = group->isModified();
    for (const Node *node : group->items()) {
        const Document *doc = m_documents.value(node->handle(), nullptr);
        modified |= doc && doc->isModified();
    }
    if (!modified) {
        return true;
    }

    QJsonArray items;
    for (const Node *node : group->items()) {
        QJsonObject item;
        node->pack(item);
        if (Document *doc = this->openDocument(node->handle())) {
            doc->pack(item);
        }
        items.append(item);
    }

    QJsonObject jGroup;
    jGroup["c:format"_L1] = "CollettDocument:1.0";
    jGroup["c:meta"_L1] = JsonUtils::packMeta(group->createdTime());
    jGroup["x:items"_L1] = items;
    if (!m_store->writeContent(group->contentName(), jGroup)) {
        m_lastError = m_store->lastError();
        return false;
    }

    // What is on disk now matches the documents, so further saves keep
    // their updated timestamps until they are edited again
    group->setModified(false);
    for (const Node *node : group->items()) {
        m_documents.value(node->handle())->setModified(false);
    }
    return true;
}

/**! @brief Load the project's user dictionary, and the dictionary for the
 * project's language, or the default language if the project has none.
 */
void Project::setupSpelling()
{
    m_spell->setStorage(m_store);
    m_spell->setLanguage(m_data->hasSpellLanguage() ? m_data->spellLanguage() : Settings::instance()->spellLanguage());
}

/**! @brief Let go of the project and everything loaded from it.
 *
 * Open documents are announced before they are deleted, so editors can let
 * go of them. The model goes away before it is deleted, so views can clear
 * themselves.
 */
void Project::releaseProject()
{
    m_autoSaveTimer->stop();
    m_countTimer->stop();
    m_countQueue.clear();

    for (Document *doc : std::as_const(m_documents)) {
        emit documentDeleting(doc);
        doc->deleteLater();
    }
    m_documents.clear();

    Storage *store = std::exchange(m_store, nullptr);
    ProjectData *data = std::exchange(m_data, nullptr);
    Tree *tree = std::exchange(m_tree, nullptr);
    const bool wasValid = std::exchange(m_isValid, false);
    m_lastError.clear();
    m_spell->setStorage(nullptr);
    if (wasValid) emit projectChanged();

    if (tree) tree->deleteLater();
    if (data) data->deleteLater();
    if (store) store->deleteLater();
}

/**! @brief Recount a document whenever its text changes.
 */
void Project::trackDocument(Document *doc)
{
    const QString handle = doc->handle();
    connect(doc, &QTextDocument::contentsChanged, this, [this, handle]() { queueCount(handle); });
}

/**! @brief Queue a document for counting. The timer is not restarted, so
 * the counts keep up while typing.
 */
void Project::queueCount(const QString &handle)
{
    m_countQueue.insert(handle);
    if (!m_countTimer->isActive()) {
        m_countTimer->start();
    }
}

} // namespace Collett
