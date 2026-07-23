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

    m_store = new Storage(path, false);
    qInfo() << "Loading Project:" << m_store->projectPath();
    if (!m_store->isValid()) {
        qWarning() << "Cannot load project from this path";
        return false;
    }

    m_data = new ProjectData();
    m_tree = new Tree(this);

    if (m_store->isNewProject()) {
        // Brand new project, nothing to read yet. Write out the initial
        // project files so they exist on disk right away.
        m_isValid = true;
        m_autoSaveTimer->setInterval(Settings::instance()->editorAutoSave() * 1000);
        m_autoSaveTimer->start();
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

    QJsonObject jData, jTree;

    m_data->pack(jData);
    if (!m_store->writeProject(jData)) {
        m_lastError = m_store->lastError();
        return false;
    }

    m_tree->pack(jTree);
    if (!m_store->writeStructure(jTree)) {
        m_lastError = m_store->lastError();
        return false;
    }

    this->saveOpenDocuments();

    return true;
}

bool Project::saveProjectAs(const QString &path)
{
    m_store = new Storage(path, false);
    m_isValid = true;
    return this->saveProject();
}

// Document Methods
// ================

/**!
 * @brief Open a document by its handle, loading or creating it as needed.
 *
 * The previously open document, if any, is saved before the new one is
 * loaded. Documents are cached for the lifetime of the project so that
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

    if (handle == m_currentDocHandle && m_documents.contains(handle)) {
        return m_documents.value(handle);
    }

    this->saveDocument(m_currentDocHandle);
    m_currentDocHandle = handle;

    if (m_documents.contains(handle)) {
        return m_documents.value(handle);
    }

    Document *doc = new Document(this);
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

    return m_store->writeDocument(handle, jDoc);
}

/**!
 * @brief Save all currently open (cached) documents to storage.
 *
 * @return bool True if all documents were saved successfully.
 */
bool Project::saveOpenDocuments()
{
    bool result = true;
    for (const QString &handle : m_documents.keys()) {
        result &= this->saveDocument(handle);
    }
    return result;
}

// Private Slots
// =============

void Project::onAutoSave()
{
    this->saveDocument(m_currentDocHandle);
}

} // namespace Collett
