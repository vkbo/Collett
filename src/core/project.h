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

#ifndef COLLETT_PROJECT_H
#define COLLETT_PROJECT_H

#include "collett.h"

#include <QObject>
#include <QVector>
#include <QString>
#include <QJsonObject>

namespace Collett {

class Project : public QObject
{
    Q_OBJECT

public:
    struct Content {
        QString path = "";
    };

    explicit Project();
    ~Project();

    // Class Methods

    void openProject(const QString &path);
    void saveProject();

    static bool jsonDocumentReader(const QString &filePath, QJsonObject &fileData);
    static bool jsonDocumentWriter(const QString &filePath, const QJsonObject &fileData, bool compact);

    // Class Setters

    void setProjectName(const QString &name);

    // Class Getters

    QString projectName() const;
    QString relativePath(const QString &path) const;
    bool hasError() const;
    QString lastError() const;

private:
    QString m_projectPath;
    QString m_lastError;
    bool    m_changed;

    // Meta Data

    QString m_created;
    QString m_updated;

    // Content

    QString m_projectName;
    QVector<Content> m_content;

    // Functions

    void setChanged(bool state);
};
} // namespace Collett

#endif // COLLETT_PROJECT_H
