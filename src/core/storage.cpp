/*
** Collett – Core Storage Class
** ============================
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

#include "storage.h"
#include "tools.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

namespace Collett {

// Constructor/Destructor
// ======================

Storage::Storage(const QString &path, bool compact) : m_compactJson(compact) {

    QFileInfo pathInfo(path);
    if (pathInfo.isFile() && pathInfo.suffix().toLower() == "collett") {
        m_isValid = pathInfo.isWritable();
    } else {
        m_isValid = false;
        qWarning() << "Invalid path:" << path;
    }
    if (m_isValid) {
        m_rootPath = pathInfo.absoluteDir();
        m_rootPath.mkdir("project");
        m_rootPath.mkdir("content");
        m_projectDir = QDir(m_rootPath.path() + "/project");
        m_contentDir = QDir(m_rootPath.path() + "/content");
    }

    qDebug() << "Root Path:" << m_rootPath.path();
};

Storage::~Storage() {
    qDebug() << "Destructor: Storage";
};

// Public Methods
// ==============

bool Storage::readProject(QJsonObject &fileData) {
    if (m_isValid) {
        return this->readJson(m_projectDir.filePath("project.json"), fileData, true);
    }
    return false;
}

bool Storage::writeProject(const QJsonObject &fileData) {
    if (m_isValid) {
        writeCollett();
        return this->writeJson(m_projectDir.filePath("project.json"), fileData);
    }
    return false;
}

bool Storage::readStructure(QJsonObject &fileData) {
    if (m_isValid) {
        return this->readJson(m_projectDir.filePath("structure.json"), fileData, false);
    }
    return false;
}

bool Storage::writeStructure(const QJsonObject &fileData) {
    if (m_isValid) {
        writeCollett();
        return this->writeJson(m_projectDir.filePath("structure.json"), fileData);
    }
    return false;
}

// Getters
// =======

QString Storage::projectPath() const {
    if (m_isValid) {
        return m_rootPath.path();
    } else {
        return QString();
    }
}

// Private Methods
// ===============

bool Storage::readJson(const QString &filePath, QJsonObject &fileData, bool required) {

    switch (JsonUtils::readJson(filePath, fileData, required)) {
        case JsonUtilsError::FileError:
            m_lastError = tr("Could not open file: %1").arg(filePath);
            return false;
        case JsonUtilsError::JsonError:
            m_lastError = tr("Could not parse file: %1").arg(filePath);
            return false;
        default:
            return true;
    }
}

bool Storage::writeJson(const QString &filePath, const QJsonObject &fileData) {

    switch (JsonUtils::writeJson(filePath, fileData, m_compactJson)) {
        case JsonUtilsError::FileError:
            m_lastError = tr("Could not open file: %1").arg(filePath);
            return false;
        case JsonUtilsError::JsonError: // N/A
        default:
            return true;
    }
}

void Storage::writeCollett() {
    QFile file(m_rootPath.filePath("CollettProject.collett"));
    if (file.open(QIODevice::WriteOnly)) {
        file.write("Collett " + QByteArray(COL_VERSION_STR));
    }
    file.close();
}

} // namespace Collett
