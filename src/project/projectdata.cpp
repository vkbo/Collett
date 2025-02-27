/*
** Collett – Project Data Class
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

#include "projectdata.h"
#include "storage.h"
#include "tools.h"

#include <QDateTime>
#include <QJsonObject>
#include <QString>

using namespace Qt::Literals::StringLiterals;

namespace Collett {

// Constructor/Destructor
// ======================

ProjectData::ProjectData(QObject *parent) : QObject(parent) {
    m_createdTime = QDateTime::currentDateTime().toString(Qt::ISODate);
}

ProjectData::~ProjectData() {
    qDebug() << "Destructor: ProjectData";
}

// Public Methods
// ==============

void ProjectData::pack(QJsonObject &data) {

    QJsonObject jMeta, jProject, jSettings;

    // Project Meta
    jMeta["m:version"_L1] = QString(COL_VERSION_STR);
    jMeta["m:created"_L1] = m_createdTime;
    jMeta["m:updated"_L1] = QDateTime::currentDateTime().toString(Qt::ISODate);

    // Project Settings
    jProject["u:name"_L1] = m_projectName;

    // Root Object
    data["c:format"_L1] = "CollettProjectData";
    data["c:meta"_L1] = jMeta;
    data["c:project"_L1] = jProject;
    data["c:settings"_L1] = jSettings;
}

void ProjectData::unpack(const QJsonObject &data) {

    QJsonObject jMeta = data.value("c:meta"_L1).toObject();
    QJsonObject jProject = data.value("c:project"_L1).toObject();
    QJsonObject jSettings = data.value("c:settings"_L1).toObject();

    // Project Meta
    m_createdTime = JsonUtils::getJsonString(jMeta, "m:created"_L1, "Unknown");

    // Project Settings
    m_projectName = JsonUtils::getJsonString(jProject, "u:name"_L1, tr("Unnamed Project"));
}

} // namespace Collett
