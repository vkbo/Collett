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
#include <QPointer>
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
        for (Group *group : m_tree->groups()) {
            this->loadGroup(group);
        }
        m_tree->model()->setDocument(this->editorDocument());
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
    m_tree->model()->setDocument(this->editorDocument());

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

    // The title page, in the same format as the content files. The model
    // picks it up from the document.
    Document::Item page;
    page.handle = m_tree->newHandle();
    page.level = ItemLevel::PageLevel;
    QJsonObject heading;
    heading["u:fmt"_L1] = "h1:ac";
    heading["u:txt"_L1] = "t|" + title;
    Document *doc = this->editorDocument();
    doc->appendItem(page, QJsonArray({heading}));
    doc->setModified(true);
    m_data->setLastEditedHandle(page.handle);

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
    m_savedContent.clear();
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

/**! @brief The text document of a group.
 */
Document *Project::document(Group *group) const
{
    return m_documents.value(group, nullptr);
}

/**! @brief The text document of the group shown in the editor.
 */
Document *Project::editorDocument() const
{
    ProjectModel *projectModel = this->model();
    return projectModel ? this->document(projectModel->group()) : nullptr;
}

/**!
 * @brief Start a new scene at a position in the editor's text.
 *
 * The text after the position goes to the new scene. The split is one edit,
 * so it can be undone.
 *
 * @param position  The position in the text to split at.
 * @return QString  The handle of the new document, or an empty string.
 */
QString Project::splitDocument(int position)
{
    Document *doc = this->editorDocument();
    if (!doc || !m_tree) {
        return QString();
    }
    Document::Item item;
    item.handle = m_tree->newHandle();
    if (doc->insertItem(position, item) < 0) {
        return QString();
    }
    return item.handle;
}

/**!
 * @brief Delete a document from the project, with its text.
 *
 * The last document in the group cannot be deleted. The deletion is an edit
 * of the text, so it can be undone.
 *
 * @param handle The handle of the document to delete.
 * @return bool  True if the document was deleted.
 */
bool Project::deleteDocument(const QString &handle)
{
    Document *doc = this->editorDocument();
    return doc && doc->removeItem(handle);
}

/**!
 * @brief Merge a document into the document before it.
 *
 * The title is removed, so the text follows the text of the previous
 * document, which keeps its title, type and other settings.
 *
 * @param handle The handle of the document to merge.
 * @return int   The position where the merged text starts, or -1 if the
 *               document could not be merged.
 */
int Project::mergeDocument(const QString &handle)
{
    Document *doc = this->editorDocument();
    return doc ? doc->mergeItem(handle) : -1;
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
    const Document *doc = this->editorDocument();
    for (const QString &handle : std::as_const(m_countQueue)) {
        const int row = projectModel->rowOf(handle);
        if (doc && row >= 0) {
            projectModel->setCounts(row, TextCounter::standardCount(doc->snapshotItem(handle)));
        }
    }
    m_countQueue.clear();
}

// Private Methods
// ===============

/**! @brief Read the documents of a group, with their text, from the
 * group's content file, into the group's text document.
 *
 * Entries that are not valid nodes are skipped, as are those with a handle
 * that is already in use. Returns false if the file could not be read.
 */
bool Project::loadGroup(Group *group)
{
    const qint64 start = QDateTime::currentMSecsSinceEpoch();
    QJsonObject jGroup;
    if (!m_store->readContent(group->contentName(), jGroup)) {
        return false;
    }
    group->setCreatedTime(JsonUtils::unpackCreated(jGroup, group->createdTime()));

    Document *doc = new Document(this);
    for (const QJsonValue &value : jGroup["x:items"_L1].toArray()) {
        const QJsonObject jItem = value.toObject();
        Node *node = Node::unpack(jItem);
        if (!node) continue;
        if (!m_tree->addNode(group, node)) {
            delete node;
            continue;
        }
        Document::Item item;
        item.handle = node->handle();
        item.title = node->title();
        item.level = node->itemLevel();
        item.hardBreak = node->hasHardBreak();
        item.numbered = node->isNumbered();
        const QJsonArray content = jItem["x:content"_L1].toArray();
        doc->appendItem(item, content);
        m_savedContent.insert(item.handle, content);
    }
    doc->setModified(false);
    m_documents.insert(group, doc);
    trackDocument(group, doc);
    group->setModified(false);

    qDebug() << "Group loaded in" << QDateTime::currentMSecsSinceEpoch() - start << "ms";
    return true;
}

/**! @brief Write a group's documents, with their text, to the group's
 * content file.
 *
 * The file is only written if the group or its text has changed since it
 * was read or last saved. A document's updated time is moved when its text
 * differs from what was last read or saved.
 */
bool Project::saveGroup(Group *group)
{
    Document *doc = this->document(group);
    if (!group->isModified() && !(doc && doc->isModified())) {
        return true;
    }

    const QString now = QDateTime::currentDateTime().toString(Qt::ISODate);
    QHash<QString, QJsonArray> saved;
    QJsonArray items;
    for (Node *node : group->items()) {
        const QJsonArray content = doc ? doc->packContent(node->handle()) : QJsonArray();
        auto it = m_savedContent.constFind(node->handle());
        if (it == m_savedContent.constEnd() || it.value() != content) {
            node->setUpdatedTime(now);
            saved.insert(node->handle(), content);
        }
        QJsonObject item;
        node->pack(item);
        item["x:content"_L1] = content;
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

    // What is on disk now matches the documents
    group->setModified(false);
    if (doc) doc->setModified(false);
    m_savedContent.insert(saved);
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
    m_savedContent.clear();
    if (m_tree) m_tree->model()->setDocument(nullptr);

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

/**! @brief Follow the changes to a group's text document.
 *
 * The documents touched by a change are recounted, and the model of the
 * group follows the structure of the text.
 */
void Project::trackDocument(Group *group, Document *doc)
{
    connect(doc, &QTextDocument::contentsChange, this, [this, doc](int position, int removed, int added) {
        Q_UNUSED(removed);
        const QString handle = doc->handleAt(position);
        if (!handle.isEmpty()) queueCount(handle);
        for (QTextBlock block = doc->findBlock(position).next(); block.isValid() && block.position() <= position + added; block = block.next()) {
            if (Document::isTitle(block)) queueCount(Document::itemOf(block).handle);
        }
    });
    connect(doc, &QTextDocument::contentsChanged, this, [this, group]() { syncGroup(group); });
}

/**! @brief Make the model follow the structure of a group's text, if the
 * group is shown.
 *
 * The text is then fixed up, if the edit left it with text before the first
 * title or with a repeated title. That is done after the edit is finished,
 * and added to the same undo step.
 */
void Project::syncGroup(Group *group)
{
    ProjectModel *projectModel = this->model();
    Document *doc = this->document(group);
    if (!projectModel || !doc || projectModel->group() != group) {
        return;
    }

    projectModel->sync(
        doc->items(),
        [this](const Document::Item &item) {
            queueCount(item.handle);
            return m_tree->createNode(item.handle, item.level);
        },
        [this](Node *node) {
            m_tree->forgetNode(node->handle());
            delete node;
        }
    );

    if (m_normalizing) return;
    m_normalizing = true;
    auto normalize = [this, doc = QPointer<Document>(doc)]() {
        m_normalizing = false;
        if (doc && m_tree) doc->normalize(m_tree->newHandle());
    };
    QMetaObject::invokeMethod(this, normalize, Qt::QueuedConnection);
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
