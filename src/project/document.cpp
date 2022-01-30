/*
** Collett – Document Class
** ========================
**
** This file is a part of Collett
** Copyright 2020–2022, Veronica Berglyd Olsen
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

#include "collett.h"
#include "document.h"

#include <QUuid>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>

namespace Collett {

Document::Document(Storage *store, const QUuid uuid, Document::Mode mode)
    : m_store(store), m_handle(uuid)
{
    m_empty = true;
    m_existing = false;
    m_unsaved = true;
    m_mode = mode;
    m_created = QDateTime::currentDateTime().toString(Qt::ISODate);
}

/**
 * Class Getters
 * =============
 */

bool Document::isEmpty() const {
    return m_empty;
}

bool Document::isExisting() const {
    return m_existing;
}

bool Document::isUnsaved() const {
    return m_unsaved;
}

QJsonArray Document::content() const {
    return m_content;
}

QUuid Document::handle() const {
    return m_handle;
}

/**
 * Class Methods
 * =============
 */

/**!
 * @brief Open the document and read the content into the data buffer.
 * 
 * @param mode either ReadOnly or ReadWrite.
 * @return true if successful, otherwise false.
 */
bool Document::open(const Document::Mode mode) {

    QJsonObject json;

    m_mode = mode;

    if (!m_store->loadFile(m_handle, json)) {
        return false;
    }

    if (json.contains(QLatin1String("m:created"))) {
        m_created = json.value(QLatin1String("m:created")).toString();
    } else {
        m_created = "unknown";
    }
    if (json.contains(QLatin1String("x:content"))) {
        m_content = json.value(QLatin1String("x:content")).toArray();
    } else {
        m_content = QJsonArray();
    }

    m_empty = false;
    m_existing = true;

    return true;
}

/**!
 * @brief Update content and save data.
 * 
 * This is a convenience function.
 * 
 * @param content the updated document content.
 * @return true if successful, otherwise false.
 */
bool Document::save(const QJsonArray &content) {
    m_content = content;
    m_empty = false;
    return save();
}

/**!
 * @brief Save the content in the data buffer to the document file.
 * 
 * @return true if successful, otherwise false.
 */
bool Document::save() {

    if (m_mode != Document::ReadWrite || m_empty) {
        return false;
    }

    QJsonObject json;
    json.insert(QLatin1String("m:created"), m_created);
    json.insert(QLatin1String("m:updated"), QDateTime::currentDateTime().toString(Qt::ISODate));
    json.insert(QLatin1String("x:content"), m_content);

    if (!m_store->saveFile(m_handle, json)) {
        return false;
    }

    m_existing = true;
    m_unsaved = false;
    return true;
}

} // namespace Collett
