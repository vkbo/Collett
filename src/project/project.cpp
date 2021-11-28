/*
Collett – Project Class
=======================

This file is a part of Collett
Copyright 2020–2021, Veronica Berglyd Olsen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "collett.h"
#include "project.h"
#include "storymodel.h"

#include <QDir>
#include <QFile>
#include <QString>
#include <QFileInfo>
#include <QIODevice>
#include <QByteArray>
#include <QTextStream>

namespace Collett {

Project::Project(const QString &path) {

    this->clearError();
    m_hasProject = false;
    m_pathValid = false;

    m_storyModel = new StoryModel(this);

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

    // Verify that the needed project folders exist
    if (!m_contentPath.exists()) {
        if (m_projectPath.mkdir("content")) {
            qDebug() << "Created folder:" << m_contentPath.path();
        } else {
            setError(tr("Could not create folder: %1").arg(m_contentPath.path()));
            return;
        }
    }

    m_pathValid = true;

    qDebug() << "Project Path:" << m_projectPath.path();
    qDebug() << "Project File:" << m_projectFile.path();
    qDebug() << "Content Path:" << m_contentPath.path();
}

Project::~Project() {
    delete m_storyModel;
}

/*
    Class Methods
    =============
*/

bool Project::openProject() {

    qInfo() << "Loading Project:" << m_projectPath.path();
    if (!m_pathValid) {
        qWarning() << "Cannot load project from this path";
        return false;
    }

    bool mainFile = loadProjectFile();

    m_hasProject = mainFile;

    return m_hasProject;
}

bool Project::saveProject() {

    qInfo() << "Saving Project:" << m_projectPath.path();
    if (!m_hasProject) {
        qWarning() << "No project open, nothing to save";
        return false;
    }

    bool mainFile = saveProjectFile();

    return mainFile;
}

/*
    Project Getters
    ===============
*/

bool Project::hasProject() const {
    return m_hasProject;
}

StoryModel *Project::storyModel() {
    return m_storyModel;
}

/*
    Main Project File
    =================
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

/*
    Error Handling
    ==============
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
