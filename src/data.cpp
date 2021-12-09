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

#include "data.h"
#include "project.h"
#include "storymodel.h"

#include <QWidget>
#include <QDir>

namespace Collett {

/**
 * Private Class Declaration
 * =========================
 */

class CollettDataPrivate
{
public:
    static CollettData *instance;

    CollettDataPrivate() {};
    ~CollettDataPrivate() {
        qDebug() << "Deconstructing: CollettDataPrivate";
        delete m_project;
    };

    Project *m_project;

    bool m_hasProject = false;
};

/**
 * Public Class Contruction/Deconstruction
 * =======================================
 */

CollettData *CollettDataPrivate::instance = nullptr;

CollettData *CollettData::instance() {
    if (CollettDataPrivate::instance == nullptr) {
        CollettDataPrivate::instance = new CollettData();
        qDebug() << "CollettData instance created";
    }
    return CollettDataPrivate::instance;
}

CollettData::CollettData() : d_ptr(new CollettDataPrivate()) {
    Q_D(CollettData);
}

CollettData::~CollettData() {}

/**
 * Public Class Methods
 * ====================
 */

bool CollettData::openProject(const QString &path) {
    Q_D(CollettData);

    d->m_project = new Project(path);
    bool status = d->m_project->openProject();
    d->m_hasProject = d->m_project->hasProject();

    return status;
}

bool CollettData::saveProject() {
    Q_D(CollettData);

    if (d->m_hasProject) {
        return d->m_project->saveProject();
    } else {
        return false;
    }
}

Project *CollettData::project() {
    Q_D(CollettData);

    if (d->m_hasProject) {
        return d->m_project;
    } else {
        return nullptr;
    }
}

StoryModel *CollettData::storyModel() {
    Q_D(CollettData);

    if (d->m_hasProject) {
        return d->m_project->storyModel();
    } else {
        return nullptr;
    }
}

} // namespace Collett
