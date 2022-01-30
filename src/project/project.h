/*
** Collett – Project Class
** =======================
**
** This file is a part of Collett
** Copyright 2020–2022, Veronica Berglyd Olsen
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
#include "document.h"
#include "storymodel.h"

#include <QDir>
#include <QHash>
#include <QUuid>
#include <QObject>
#include <QString>

namespace Collett {

class Project : public QObject
{
    Q_OBJECT

public:
    explicit Project(const QString &path);
    ~Project();

    // Class Methods

    bool openProject();
    bool saveProject();
    bool isValid() const;

    // Class Setters

    void setLastDocumentMain(const QUuid &uuid);
    void setProjectName(const QString &name);

    // Class Getters

    QUuid lastDocumentMain() const;

    QString projectName() const;
    StoryModel *storyModel();
    Storage *store();

    Document *document(const QUuid &uuid);

    // Error Handling

    bool hasError() const;
    QString lastError() const;

private:
    bool     m_isValid;
    QString  m_lastError;
    Storage *m_store;

    // Project Meta

    QString m_collettVersion = "";
    QString m_projectVersion = "";
    QString m_createdTime = "";

    // Project State

    QUuid m_lastDocMain;

    // Project Settings

    QString m_projectName = "New Project";

    // Content

    StoryModel *m_storyModel;
    QHash<QUuid, Document*> m_documents;

    // File Load & Save

    bool loadSettingsFile();
    bool saveSettingsFile();

    bool loadStoryFile();
    bool saveStoryFile();

    void loadDocuments();
    void saveDocuments();

};
} // namespace Collett

#endif // COLLETT_PROJECT_H
