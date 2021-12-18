/*
** Collett – Core Storage Class
** ============================
**
** This file is a part of Collett
** Copyright 2020–2021, Veronica Berglyd Olsen
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
#include "storage.h"

#include <QDir>
#include <QFile>
#include <QUuid>
#include <QString>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>

namespace Collett {

Storage::Storage(const QString &path, Mode mode, bool compact)
    : m_saveMode(mode), m_compactJson(compact)
{
    QFileInfo pathInfo(path);
    switch (m_saveMode) {

    case Mode::Folder:
        m_rootPath = QDir(QDir(path).absolutePath());

        if (!pathInfo.exists()) {
            QDir parDir = pathInfo.dir();
            if (parDir.exists()) {
                m_isValid = m_rootPath.mkpath(".");
            }
        } else {
            m_isValid = pathInfo.isDir();
        }

        if (!m_isValid) {
            m_lastError = tr("Could not find or create project storage folder: %1").arg(path);
        }
        break;
    
    case Mode::Archive:
        m_lastError = "Archive storage is not yet implemented";
        break;
    }

    qDebug() << "Root Path:" << m_rootPath.path();
};

Storage::~Storage() {
    qDebug() << "Destructor: Storage";
};

/**
 * Class Methods
 */

bool Storage::loadFile(const QString &fileName, QJsonObject &fileData) {
    if (!ensureFolder("project")) {
        return false;
    }
    QString filePath = QDir(m_rootPath.path() + "/project").filePath(fileName + ".json");
    return readJson(filePath, fileData);
}

bool Storage::loadFile(const QUuid &fileUuid, QJsonObject &fileData) {
    return true;
}

bool Storage::saveFile(const QString &fileName, const QJsonObject &fileData) {
    if (!ensureFolder("project")) {
        return false;
    }
    QString filePath = QDir(m_rootPath.path() + "/project").filePath(fileName + ".json");
    return writeJson(filePath, fileData);
}

bool Storage::saveFile(const QUuid &fileUuid, const QJsonObject &fileData) {
    return true;
}

bool Storage::loadProjectFile() {

    if (!m_isValid) {
        return false;
    }

    bool hasCol = false;
    bool hasPro = false;

    QFile prjFile(m_rootPath.filePath("project.collett"));
    if (prjFile.open(QIODevice::ReadOnly)) {
        QString line;
        QTextStream inStream(&prjFile);
        while (!inStream.atEnd()) {
            line = inStream.readLine();
            if (line.startsWith("Collett ")) {
                m_collettVersion = line.remove(0, 8);
                hasCol = true;
            } else if (line.startsWith("Project ")) {
                m_projectVersion = line.remove(0, 8);
                hasPro = true;
            }
        }
        prjFile.close();
        qInfo() << "File Collett Version:" << m_collettVersion;
        qInfo() << "File Project Version:" << m_projectVersion;
        return hasCol & hasPro;
    } else {
        return false;
    }
}

bool Storage::saveProjectFile() {

    if (!m_isValid) {
        return false;
    }

    QByteArray colLine = "Collett " + QByteArray(COL_VERSION_STR);
    QByteArray proLine = "Project 0.1";

    QFile prjFile(m_rootPath.filePath("project.collett"));
    if (prjFile.open(QIODevice::WriteOnly)) {
        QTextStream outData(&prjFile);
        outData << "Collett " << QString(COL_VERSION_STR) << Qt::endl;
        outData << "Project 0.1" << Qt::endl;
        prjFile.close();
        return true;
    } else {
        return false;
    }
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
QString Storage::getJsonString(const QJsonObject &object, const QString &key, QString def) {
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
    return true;
}

bool Storage::writeJson(const QString &filePath, const QJsonObject &fileData) {

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        m_lastError = tr("Could not open file: %1").arg(filePath);
        qWarning() << "Could not open file:" << filePath;
        return false;
    }

    QJsonDocument doc(fileData);
    file.write(doc.toJson());
    file.close();
    return true;
}

bool Storage::ensureFolder(const QString &folder) {
    if (!m_isValid) {
        return false;
    }
    QDir checkPath = QDir(m_rootPath.path() + "/" + folder);
    if (checkPath.exists()) {
        return true;
    }
    if (m_rootPath.mkdir(folder)) {
        qDebug() << "Created folder:" << folder;
        return true;
    } else {
        m_lastError = tr("Could not create folder: %1").arg(folder);
        qWarning() << "Could not create folder:" << folder;
        return false;
    }
}

} // namespace Collett
