/*
** Collett – Project Class
** =======================
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
#include "project.h"
#include "storymodel.h"
#include "datautils.h"

#include <QDir>
#include <QFile>
#include <QString>
#include <QDateTime>
#include <QFileInfo>
#include <QIODevice>
#include <QByteArray>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonParseError>

namespace Collett {

/**
 * Class Constructor/Destructor
 * ============================
 */

Project::Project(const QString &path) {

    this->clearError();
    m_isValid = false;
    m_pathValid = false;
    m_storyModel = new StoryModel(this);
    m_createdTime = QDateTime::currentDateTime().toString(Qt::ISODate);

    QFileInfo fObj(path);
    if (!fObj.exists()) {
        setError(tr("Project not found at: %1").arg(path));
        return;
    }

    // If the path is a file, go one level up
    QDir projDir = QDir(path).absolutePath();
    if (fObj.isFile()) {
        projDir = fObj.dir().absolutePath();
    }

    QDir projFile = projDir.filePath(COL_PROJECT_FILE_NAME);
    if (!QFileInfo::exists(projFile.path())) {
        setError(tr("Project not found at: %1").arg(projFile.path()));
        return;
    }

    // Set the path variables
    m_projectPath = projDir;
    m_projectFile = projFile;
    m_contentPath = QDir(m_projectPath.path() + "/content");
    m_dataPath    = QDir(m_projectPath.path() + "/data");

    // Verify that the needed project folders exist
    if (!m_contentPath.exists()) {
        if (m_projectPath.mkdir("content")) {
            qDebug() << "Created folder:" << m_contentPath.path();
        } else {
            setError(tr("Could not create folder: %1").arg(m_contentPath.path()));
            return;
        }
    }
    if (!m_dataPath.exists()) {
        if (m_projectPath.mkdir("data")) {
            qDebug() << "Created folder:" << m_dataPath.path();
        } else {
            setError(tr("Could not create folder: %1").arg(m_dataPath.path()));
            return;
        }
    }

    m_pathValid = true;

    qDebug() << "Project Path:" << m_projectPath.path();
    qDebug() << "Project File:" << m_projectFile.path();
}

Project::~Project() {
    qDebug() << "Destructor: Project";
    delete m_storyModel;
}

/**
 * Class Methods
 * =============
 */

bool Project::openProject() {

    qInfo() << "Loading Project:" << m_projectPath.path();
    if (!m_pathValid) {
        qWarning() << "Cannot load project from this path";
        return false;
    }

    bool main = loadProjectFile();
    if (main) {
        bool settings = loadSettingsFile();
        bool story = loadStoryFile();
        m_isValid = settings & story;
    } else {
        m_isValid = false;
    }

    return m_isValid;
}

bool Project::saveProject() {

    qInfo() << "Saving Project:" << m_projectPath.path();
    if (!m_isValid) {
        qWarning() << "No project open, nothing to save";
        return false;
    }

    bool mainFile = saveProjectFile();
    bool settFile = saveSettingsFile();
    bool storyFile = saveStoryFile();

    return mainFile & settFile & storyFile;
}

/**
 * Class Setters
 * =============
 */

void Project::setProjectName(const QString &name) {
    m_projectName = name.simplified();
}

/**
 * Class Getters
 * =============
 */

QString Project::projectName() const {
    return m_projectName;
}

bool Project::isValid() const {
    return m_isValid;
}

StoryModel *Project::storyModel() {
    return m_storyModel;
}

/**
 * Project File
 * ============
 * Load and save functions for the project.collett file.
 */

bool Project::loadProjectFile() {

    bool hasCol = false;
    bool hasPro = false;

    QFile prjFile(m_projectFile.path());
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

bool Project::saveProjectFile() {

    QByteArray colLine = "Collett " + QByteArray(COL_VERSION_STR);
    QByteArray proLine = "Project 0.1";

    QFile prjFile(m_projectFile.path());
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

/**
 * Settings FIle
 * =============
 * Load and save functions for the data/project.json file.
 *
 * This file contains all the meta data and options for the Collett project,
 * except for the project content itself (the documents).
 */

bool Project::loadSettingsFile() {

    QFile file(m_dataPath.filePath(COL_SETTINGS_FILE_NAME));
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open settings file";
        return false;
    }

    QJsonParseError *error = new QJsonParseError();
    QJsonDocument json = QJsonDocument::fromJson(file.readAll(), error);
    if (error->error != QJsonParseError::NoError) {
        qWarning() << "Could not parse story file";
        qWarning() << error->errorString();
        return false;
    }

    if (!json.isObject()) {
        qWarning() << "Unexpected content of settings file";
        return false;
    }
    QJsonObject jData = json.object();
    QJsonObject jMeta = jData["meta"].toObject();
    QJsonObject jProject = jData["project"].toObject();
    QJsonObject jSettings = jData["settings"].toObject();

    m_createdTime = DataUtils::getJsonString(jMeta, "created", "Unknown");
    m_projectName = DataUtils::getJsonString(jProject, "projectName", tr("Unnamed Project"));

    return true;
}

bool Project::saveSettingsFile() {

    QJsonObject jData, jMeta, jProject, jSettings;

    jMeta["created"] = m_createdTime;
    jMeta["updated"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    jProject["projectName"] = m_projectName;

    jData["meta"] = jMeta;
    jData["project"] = jProject;
    jData["settings"] = jSettings;

    QFile file(m_dataPath.filePath(COL_SETTINGS_FILE_NAME));
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open settings file";
        return false;
    }

    QJsonDocument doc(jData);
    file.write(doc.toJson());
    file.close();

    return true;
}

/**
 * Story File
 * ==========
 * Load and save functions for the data/story.json file.
 *
 * This file contains the structure of StoryItems contained in the StoryModel.
 * The structure is contained as child items under a single root item, and is
 * saved to a QJsonDocument by recursively calling the StoryItem->toJsonObject
 * function.
`*/

bool Project::loadStoryFile() {

    QFile file(m_dataPath.filePath(COL_STORY_FILE_NAME));
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open story file";
        return false;
    }

    QJsonParseError *error = new QJsonParseError();
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), error);
    if (error->error != QJsonParseError::NoError) {
        qWarning() << "Could not parse story file";
        qWarning() << error->errorString();
        return false;
    }

    if (doc.isObject()) {
        return m_storyModel->fromJsonObject(doc.object());
    } else {
        qWarning() << "Unexpected content of story file";
        return false;
    }
}

bool Project::saveStoryFile() {

    QFile file(m_dataPath.filePath(COL_STORY_FILE_NAME));
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open story file";
        return false;
    }

    QJsonDocument doc(m_storyModel->toJsonObject());
    file.write(doc.toJson());
    file.close();

    return true;
}

/**
 * Error Handling
 * ==============
 */

bool Project::hasError() const {
    return m_hasError;
}

QString Project::lastError() const {
    return m_lastError;
}

void Project::clearError() {
    m_hasError = false;
    m_lastError = "";
}

void Project::setError(const QString &error) {
    m_hasError = true;
    m_lastError = error;
    qCritical() << error;
}

} // namespace Collett
