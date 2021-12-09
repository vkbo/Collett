/*
** Collett – Core Data Class
** =========================
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

#ifndef COLLETT_DATA_H
#define COLLETT_DATA_H

#include "collett.h"
#include "project.h"
#include "storymodel.h"

#include <QWidget>
#include <QObject>
#include <QString>
#include <QDir>

namespace Collett {

class CollettDataPrivate;
class CollettData : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(CollettData)

public:
    static CollettData *instance();
    ~CollettData();

private:
    QScopedPointer<CollettDataPrivate> d_ptr;
    CollettData();

public:
    bool openProject(const QString &path);
    bool saveProject();

    Project    *project();
    StoryModel *storyModel();

};
} // namespace Collett

#endif // COLLETT_DATA_H
