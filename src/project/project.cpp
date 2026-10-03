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

#include <QDateTime>
#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCursor>
#include <QTextDocumentFragment>
#include <QUrl>

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

    m_isValid = true;
    m_autoSaveTimer->setInterval(Settings::instance()->editorAutoSave() * 1000);
    m_autoSaveTimer->start();
    this->setupSpelling();
    emit projectChanged();

    return true;
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

    // The title page, in the same format as the document files
    Node *node = m_tree->createNode(ItemLevel::PageLevel);
    Document *doc = new Document(node->handle(), this);
    QJsonObject meta;
    meta["m:created"_L1] = QDateTime::currentDateTime().toString(Qt::ISODate);
    QJsonObject heading;
    heading["u:fmt"_L1] = "h1:ac";
    heading["u:txt"_L1] = "t|" + title;
    QJsonObject data;
    data["c:meta"_L1] = meta;
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

    return this->saveOpenDocuments();
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
    if (m_tree) m_tree->setModified(true);
    for (Document *doc : std::as_const(m_documents)) {
        doc->setModified(true);
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
 * @brief Open a document by its handle, loading or creating it as needed.
 *
 * Several documents are open at the same time when the editor shows a stack
 * of scenes. Documents are cached for the lifetime of the project so that
 * switching back to a previously opened document does not require a
 * round-trip to disk.
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
    QJsonObject jDoc;
    if (m_store->readDocument(handle, jDoc) && !jDoc.isEmpty()) {
        doc->unpack(jDoc);
    }
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
 * @brief Delete a document from the project, along with its file.
 *
 * The last document in the group cannot be deleted. The document object is
 * deleted on the next pass of the event loop, as the editor showing it is
 * only destroyed then.
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
        doc->deleteLater();
    }
    if (m_store) {
        m_store->deleteDocument(handle);
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

/**!
 * @brief Save a single open document to storage.
 *
 * @param handle The handle of the document to save.
 * @return bool  True if the document was saved, or there was nothing to save.
 */
bool Project::saveDocument(const QString &handle)
{
    if (!m_store || handle.isEmpty() || !m_documents.contains(handle)) {
        return false;
    }

    Document *doc = m_documents.value(handle);
    QJsonObject jDoc;
    doc->pack(jDoc);

    if (!m_store->writeDocument(handle, jDoc)) {
        return false;
    }

    // What is on disk now matches the document, so further saves keep its
    // updated timestamp until it is edited again
    doc->setModified(false);
    return true;
}

/**!
 * @brief Save the open (cached) documents that have changed to storage.
 *
 * A document is modified when its text has changed since it was read or
 * last saved, so unchanged documents are not written.
 *
 * @return bool True if all changed documents were saved successfully.
 */
bool Project::saveOpenDocuments()
{
    bool result = true;
    for (auto it = m_documents.cbegin(); it != m_documents.cend(); ++it) {
        if (it.value()->isModified()) {
            result &= this->saveDocument(it.key());
        }
    }
    return result;
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

/**! @brief Load the project's user dictionary, and the dictionary for the
 * project's language, or the default language if the project has none.
 */
void Project::setupSpelling()
{
    m_spell->setStorage(m_store);
    m_spell->setLanguage(m_data->hasSpellLanguage() ? m_data->spellLanguage() : Settings::instance()->spellLanguage());
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
