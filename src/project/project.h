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

#ifndef COLLETT_PROJECT_H
#define COLLETT_PROJECT_H

#include "collett.h"
#include "storage.h"
#include "storymodel.h"

#include <QDir>
#include <QHash>
#include <QObject>
#include <QString>

namespace Collett {

class ProjectXmlReader;
class Project : public QObject
{
    Q_OBJECT

public:
    Project(const QString &path);
    ~Project();

    // Class Methods

    bool openProject();
    bool isValid() const;
    void report() {
        qDebug() << "Meta:" << m_projectMeta;
        qDebug() << "Settings:" << m_projectSettings;
    };

    // Class Setters

    void setProjectName(const QString &name);

    // Class Getters (Meta)

    QString createdTime() const;

    // Class Getters (Settings)

    QString projectPath() const;
    QString projectName() const;
    StoryModel *storyModel();

    // Error Handling

    bool hasError() const;
    QString lastError() const;

private:
    bool    m_isValid;
    QString m_lastError;
    Storage *m_store;

    QHash<QString, QString> m_projectMeta;
    QHash<QString, QString> m_projectSettings;

    // Project Meta

    QString m_collettVersion = "";
    QString m_projectVersion = "";
    QString m_createdTime = "";

    // Project Settings

    QString m_projectPath;
    QString m_projectFile = "sample.collett";
    QString m_projectName = "New Project";

    // Content

    StoryModel *m_storyModel;

    // File Load & Save

    bool loadSettingsFile();
    bool loadStoryFile();

    friend class ProjectXmlReader;

};
} // namespace Collett

#endif // COLLETT_PROJECT_H
