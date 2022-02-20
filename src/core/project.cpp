/*
** Collett – Project Class
** =======================
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

#include "utils.h"
#include "collett.h"
#include "project.h"

#include <QString>
#include <QDateTime>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QApplication>
#include <QLatin1String>

namespace Collett {

/**
 * Class Constructor/Destructor
 * ============================
 */

Project::Project() {

    qDebug() << "Constructor: Project";

    m_projectPath = "";
    m_lastError = "";
    m_changed = false;

    m_created = QDateTime::currentDateTime().toString(Qt::ISODate);
    m_updated = m_created;

    m_projectName = tr("Unnamed Project");
}

Project::~Project() {
    qDebug() << "Destructor: Project";
}

/**
 * Class Methods
 * =============
 */

void Project::openProject(const QString &path) {

    QFileInfo fInfo(path);
    if (!fInfo.isFile()) {
        m_lastError = tr("The provided path is not a file.");
        return;
    }
    if (fInfo.suffix() != "collett-project") {
        m_lastError = tr("The file is not a Collett project file.");
        return;
    }

    QJsonObject json;
    if (!jsonDocumentReader(path, json)) {
        m_lastError = tr("Could not read file: %1").arg(path);
        return;
    }
    m_projectPath = path;
    qInfo() << "Loading project from:" << m_projectPath;

    QString fileFormat;
    if (json.contains(QLatin1String("$:format"))) {
        fileFormat = json.value(QLatin1String("$:format")).toString();
    }
    if (json.contains(QLatin1String("m:created"))) {
        m_created = json.value(QLatin1String("m:created")).toString();
    } else {
        m_created = "unknown";
    }
    if (json.contains(QLatin1String("m:updated"))) {
        m_updated = json.value(QLatin1String("m:updated")).toString();
    } else {
        m_created = "unknown";
    }
    if (json.contains(QLatin1String("u:project-name"))) {
        m_projectName = json.value(QLatin1String("u:project-name")).toString();
    } else {
        m_projectName = tr("Unnamed Project");
    }
}

void Project::saveProject() {

    if (m_projectPath.isEmpty()) {
        return;
    }

    qInfo() << "Saving project to:" << m_projectPath;

    QJsonObject json;
    QJsonArray jContent;

    json.insert(QLatin1String("$:format"), "CollettProject:0.1");
    json.insert(QLatin1String("$:version"), qApp->applicationVersion());
    json.insert(QLatin1String("m:created"), m_created);
    json.insert(QLatin1String("m:updated"), m_updated);
    json.insert(QLatin1String("u:project-name"), m_projectName);
    json.insert(QLatin1String("x:content"), jContent);

    if (!jsonDocumentWriter(m_projectPath, json, false)) {
        m_lastError = tr("Could not write file: %1").arg(m_projectPath);
    }
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

bool Project::hasError() const {
    return !m_lastError.isEmpty();
}

QString Project::lastError() const {
    return m_lastError;
}

} // namespace Collett
