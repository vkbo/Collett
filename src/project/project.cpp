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

#include "project.h"
#include "settings.h"
#include "storage.h"

#include <QDateTime>
#include <QJsonObject>

namespace Collett {

// Constructor/Destructor
// ======================

Project::Project()
{
    m_autoSaveTimer = new QTimer(this);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &Project::onAutoSave);
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
    emit projectChanged();

    return true;
}

bool Project::saveProject()
{

    if (m_store == nullptr || m_data == nullptr) {
        qWarning() << "Project storage not initialised, cannot save";
        return false;
    }

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

    return doc;
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

} // namespace Collett
