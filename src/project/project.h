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
#include "storymodel.h"

#include <QDir>
#include <QObject>
#include <QString>

namespace Collett {

class Project : public QObject
{
    Q_OBJECT

public:
    Project(const QString &path);
    ~Project();

    // Class Methods

    bool openProject();
    bool saveProject();

    // Class Setters

    void setProjectName(const QString &name);

    // Class Getters

    QString projectName() const;
    bool isValid() const;
    StoryModel *storyModel();

    // Error Handling

    bool hasError() const;
    void clearError();
    QString lastError() const;

private:
    bool    m_isValid;
    bool    m_hasError;
    QString m_lastError;

    // Project Paths

    QDir m_projectPath;
    QDir m_projectFile;
    QDir m_contentPath;
    QDir m_dataPath;
    bool m_pathValid;

    // Project Meta

    QString m_collettVersion = "";
    QString m_projectVersion = "";
    QString m_createdTime = "";

    // Project Details

    QString m_projectName = "New Project";

    // Content

    StoryModel *m_storyModel;

    // File Load & Save

    bool loadProjectFile();
    bool saveProjectFile();
    bool loadSettingsFile();
    bool saveSettingsFile();
    bool loadStoryFile();
    bool saveStoryFile();

    // Error Handling

    void setError(const QString &error);

};
} // namespace Collett

#endif // COLLETT_PROJECT_H
