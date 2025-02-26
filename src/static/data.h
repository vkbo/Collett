/*
** Collett – Core Data Class
** =========================
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

#ifndef COLLETT_DATA_H
#define COLLETT_DATA_H

#include "collett.h"
#include "project.h"

#include <QScopedPointer>

namespace Collett {

class CollettData : public QObject
{
    Q_OBJECT

public:
    static CollettData *instance();

    CollettData(QObject *parent = nullptr);
    ~CollettData();

    // Methods
    bool openProject(const QString &path);
    bool saveProject();
    bool saveProjectAs(const QString &path);
    void closeProject();

    // Getters
    bool hasProject() const;
    Project *project();

private:
    static CollettData *staticInstance;
    QScopedPointer<Project> m_project;

};
} // namespace Collett

#endif // COLLETT_DATA_H
