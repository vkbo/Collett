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
        qDebug() << "Destructor: CollettDataPrivate";
        m_project.reset();
    };

    bool isValid() const {
        if (m_project.isNull()) {
            return false;
        } else {
            return m_project.data()->isValid();
        }
    }

    Project *project() const {
        return m_project.data();
    }

    void newProject(const QString &path) {
        m_project.reset(new Project(path));
    }

    void clearProject() {
        m_project.reset(nullptr);
    }

private:
    QScopedPointer<Project> m_project;

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

CollettData::~CollettData() {
    qDebug() << "Destructor: CollettData";
}

/**
 * Public Class Methods
 * ====================
 */

bool CollettData::openProject(const QString &path) {
    Q_D(CollettData);

    d->newProject(path);
    if (!d->project()->hasError()) {
        d->project()->openProject();
    }
    if (!d->project()->isValid()) {
        d->clearProject();
        return false;
    }

    return true;
}

bool CollettData::saveProject() {
    Q_D(CollettData);
    if (d->isValid()) {
        return d->project()->saveProject();
    } else {
        return false;
    }
}

void CollettData::closeProject() {
    Q_D(CollettData);
    d->clearProject();
}

/**
 * Public Class Getters
 * ====================
 */

bool CollettData::hasProject() const {
    Q_D(const CollettData);
    return d->isValid();
}

QVariant CollettData::projectValue(const QString &key) const {
    Q_D(const CollettData);
    if (d->isValid()) {
        return d->project()->projectValue(key);
    } else {
        return QVariant();
    }
}

StoryModel *CollettData::storyModel() {
    Q_D(CollettData);
    if (d->isValid()) {
        return d->project()->storyModel();
    } else {
        return nullptr;
    }
}

} // namespace Collett
