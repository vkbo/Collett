/*
** Collett - Project Data Class
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
#include <QJsonValue>
#include <QString>

using namespace Qt::Literals::StringLiterals;

namespace Collett {

// Constructor/Destructor
// ======================

ProjectData::ProjectData(QObject *parent) : QObject(parent)
{
    m_createdTime = QDateTime::currentDateTime().toString(Qt::ISODate);
}

ProjectData::~ProjectData()
{
    qDebug() << "Destructor: ProjectData";
}

// Public Methods
// ==============

void ProjectData::pack(QJsonObject &data)
{

    QJsonObject jProject, jSettings;

    // Project Settings
    jProject["u:name"_L1] = m_projectName;

    // Editor Settings
    jSettings["m:lastEdited"_L1] = m_lastEditedHandle;

    // Spell Check Settings
    jSettings["u:spellLanguage"_L1] = m_spellLanguage.isEmpty() ? QJsonValue() : QJsonValue(m_spellLanguage);

    // Root Object
    data["c:format"_L1] = "CollettProjectData:1.0";
    data["c:meta"_L1] = JsonUtils::packMeta(m_createdTime);
    data["c:project"_L1] = jProject;
    data["c:settings"_L1] = jSettings;
}

void ProjectData::unpack(const QJsonObject &data)
{

    QJsonObject jProject = data.value("c:project"_L1).toObject();
    QJsonObject jSettings = data.value("c:settings"_L1).toObject();

    // Project Meta
    m_createdTime = JsonUtils::unpackCreated(data, "Unknown");

    // Project Settings
    m_projectName = JsonUtils::getJsonString(jProject, "u:name"_L1, tr("Unnamed Project"));

    // Editor Settings
    m_lastEditedHandle = JsonUtils::getJsonString(jSettings, "m:lastEdited"_L1, "");

    // Spell Check Settings
    // A null or missing value means the global setting applies
    this->setSpellLanguage(jSettings.value("u:spellLanguage"_L1).toString());

    // What was read matches what is on disk
    m_modified = false;
}

// Setters
// =======

void ProjectData::setName(const QString &name)
{
    const QString simplified = name.simplified();
    if (simplified == m_projectName) return;
    m_projectName = simplified;
    m_modified = true;
}

void ProjectData::setLastEditedHandle(const QString &handle)
{
    if (handle == m_lastEditedHandle) return;
    m_lastEditedHandle = handle;
    m_modified = true;
}

void ProjectData::setSpellLanguage(const QString &language)
{
    const QString trimmed = language.trimmed();
    if (trimmed == m_spellLanguage) return;
    m_spellLanguage = trimmed;
    m_modified = true;
}

} // namespace Collett
