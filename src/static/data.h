/*
** Collett – Shared Data Class
** ===========================
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

#ifndef COLLETT_SHARED_DATA_H
#define COLLETT_SHARED_DATA_H

#include "collett.h"
#include "project.h"

#include <QScopedPointer>

namespace Collett {

class SharedData : public QObject
{
    Q_OBJECT

public:
    static SharedData *instance();

    SharedData(QObject *parent = nullptr);
    ~SharedData();

    // Methods
    bool openProject(const QString &path);
    bool saveProject();
    bool saveProjectAs(const QString &path);
    void closeProject();

    // Getters
    bool hasProject() const;
    Project *project();

signals:

    void projectLoaded();

private:
    static SharedData *staticInstance;
    QScopedPointer<Project> m_project;

};
} // namespace Collett

#endif // COLLETT_SHARED_DATA_H
