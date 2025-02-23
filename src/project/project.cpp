/*
** Collett – Project Class
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
#include "storage.h"

#include <QDateTime>
#include <QJsonObject>

namespace Collett {

/**
 * Constructor/Destructor
 */

Project::Project() {
}

Project::~Project() {
    qDebug() << "Destructor: Project";
}

/**
 * Public Methods
 */

bool Project::openProject(const QString &path) {

    m_store = new Storage(path, false);
    qInfo() << "Loading Project:" << m_store->projectPath();
    if (!m_store->isValid()) {
        qWarning() << "Cannot load project from this path";
        return false;
    }

    QJsonObject jData;
    if (!m_store->readProject(jData)) {
        m_lastError = m_store->lastError();
        return false;
    }

    m_isValid = true;

    return true;
}

bool Project::saveProject() {

    if (m_store == nullptr) {
        qWarning() << "Project storage not initialised, cannot save";
        return false;
    }

    qInfo() << "Saving Project:" << m_store->projectPath();
    if (!m_store->isValid()) {
        qWarning() << "Project storage invalid, cannot save";
        return false;
    }

    QJsonObject jData;

    if (!m_store->writeProject(jData)) {
        m_lastError = m_store->lastError();
        return false;
    }

    return true;
}

bool Project::saveProjectAs(const QString &path) {
    m_store = new Storage(path, false);
    m_isValid = true;
    return this->saveProject();
}

/**
 * Getters
 */

bool Project::isValid() const {
    return m_isValid;
}

Storage *Project::store() {
    return m_store;
}

/**
 * Error Handling
 */

bool Project::hasError() const {
    return !m_lastError.isEmpty();
}

QString Project::lastError() const {
    return m_lastError;
}

} // namespace Collett
