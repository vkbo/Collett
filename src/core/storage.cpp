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

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

namespace Collett {

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

/**
 * Public Methods
 */

bool Storage::readProject(QJsonObject &fileData) {
    if (m_isValid) {
        return this->readJson(m_projectDir.filePath("project.json"), fileData);
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

bool Storage::isValid() {
    return m_isValid;
}

QString Storage::projectPath() const {
    if (m_isValid) {
        return m_rootPath.path();
    } else {
        return QString();
    }
}

bool Storage::hasError() {
    return !m_lastError.isEmpty();
}

QString Storage::lastError() const {
    return m_lastError;
}

/**
 * Static Methods
 */

/**!
 * @brief Get string from JSON object.
 *
 * @param object the json object.
 * @param key    the key of the value to look up.
 * @param def    the default value to return in case the key does not exist.
 * @return the value or the default as a string.
 */
QString Storage::getJsonString(const QJsonObject &object, const QLatin1String &key, QString def) {
    if (object.contains(key)) {
        return object.value(key).toString();
    } else {
        return def;
    }
}

/**
 * Private Methods
 */

bool Storage::readJson(const QString &filePath, QJsonObject &fileData) {

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = tr("Could not open file: %1").arg(filePath);
        qWarning() << "Could not open file:" << filePath;
        return false;
    }

    QJsonParseError *error = new QJsonParseError();
    QJsonDocument json = QJsonDocument::fromJson(file.readAll(), error);
    if (error->error != QJsonParseError::NoError) {
        m_lastError = tr("Could not parse file: %1").arg(filePath);
        qWarning() << "Could not parse file:" << filePath;
        qWarning() << error->errorString();
        return false;
    }

    if (!json.isObject()) {
        m_lastError = tr("Unexpected content of file: %1").arg(filePath);
        qWarning() << "Unexpected content of file:" << filePath;
        return false;
    }

    fileData = json.object();
    qDebug() << "Read:" << filePath;

    return true;
}

bool Storage::writeJson(const QString &filePath, const QJsonObject &fileData) {

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        m_lastError = tr("Could not open file: %1").arg(filePath);
        qWarning() << "Could not open file:" << filePath;
        return false;
    }
    file.write(QJsonDocument(fileData).toJson(
        m_compactJson ? QJsonDocument::Compact : QJsonDocument::Indented
    ));
    file.close();
    qDebug() << "Wrote:" << filePath;
    return true;
}

void Storage::writeCollett() {
    QFile file(m_rootPath.filePath("CollettProject.collett"));
    if (file.open(QIODevice::WriteOnly)) {
        file.write("Collett");
    }
    file.close();
}

} // namespace Collett
