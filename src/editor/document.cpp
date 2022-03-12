/*
** Collett – Document Class
** ========================
**
** This file is a part of Collett
** Copyright 2021–2022, Veronica Berglyd Olsen
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
#include "collection.h"
#include "document.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>

namespace Collett {

Document::Document(const QString &path) : m_path(path) {

    m_locked = false;
    m_unsaved = true;
    m_created = QDateTime::currentDateTime().toString(Qt::ISODate);
    m_updated = QDateTime::currentDateTime().toString(Qt::ISODate);
}

/**
 * Class Setters
 * =============
 */

void Document::setContent(const QJsonArray &content) {
    m_content = content;
    this->docChanged();
}

void Document::setLocked(bool locked) {
    m_locked = locked;
}

/**
 * Class Getters
 * =============
 */

QJsonArray Document::content() const {
    return m_content;
}

/**
 * Class State
 * ===========
 */

bool Document::isEmpty() const {
    return m_content.isEmpty();
}

bool Document::isUnsaved() const {
    return m_unsaved;
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
bool Document::read() {

    QJsonObject json;

    if (!Collection::jsonDocumentReader(m_path, json)) {
        return false;
    }

    if (json.contains(QLatin1String("m:created"))) {
        m_created = json.value(QLatin1String("m:created")).toString();
    } else {
        m_created = "unknown";
    }
    if (json.contains(QLatin1String("m:created"))) {
        m_updated = json.value(QLatin1String("m:updated")).toString();
    } else {
        m_updated = "unknown";
    }
    if (json.contains(QLatin1String("x:content"))) {
        m_content = json.value(QLatin1String("x:content")).toArray();
    } else {
        m_content = QJsonArray();
    }

    m_unsaved = false;

    return true;
}

/**!
 * @brief Save the content in the data buffer to the document file.
 *
 * @return true if successful, otherwise false.
 */
bool Document::write() {

    if (isEmpty()) {
        // Nothing to do
        m_unsaved = false;
        return true;
    }

    if (m_locked) {
        qInfo() << "Document is locked";
        return false;
    }

    QJsonObject json;
    json.insert(QLatin1String("c:format"), "document");
    json.insert(QLatin1String("m:created"), m_created);
    json.insert(QLatin1String("m:updated"), m_updated);
    json.insert(QLatin1String("x:content"), m_content);

    if (!Collection::jsonDocumentWriter(m_path, json, false)) {
        return false;
    }

    m_unsaved = false;

    return true;
}

/**
 * Internal Functions
 * ==================
 */

void Document::docChanged() {
    m_updated = QDateTime::currentDateTime().toString(Qt::ISODate);
    m_unsaved = true;
}

} // namespace Collett
