/*
** Collett – Core Data Class
** =========================
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

#include "data.h"
#include "project.h"
#include "storymodel.h"

#include <QString>
#include <QWidget>
#include <QVariant>

namespace Collett {

/**
 * Class Constructor/Destructor/Instance
 * =====================================
 */

CollettData *CollettData::staticInstance = nullptr;
CollettData *CollettData::instance() {
    if (staticInstance == nullptr) {
        staticInstance = new CollettData();
        qDebug() << "Constructor: CollettData";
    }
    return staticInstance;
}

CollettData::CollettData() {}
CollettData::~CollettData() {
    qDebug() << "Destructor: CollettData";
    m_project.reset();
}

/**
 * Class Methods
 * =============
 */

bool CollettData::openProject(const QString &path) {

    m_project.reset(new Project(path));
    if (!m_project.data()->hasError()) {
        m_project.data()->openProject();
    }
    if (!m_project.data()->isValid()) {
        m_project.reset(nullptr);
        return false;
    }

    return true;
}

bool CollettData::saveProject() {
    if (hasProject()) {
        return m_project.data()->saveProject();
    } else {
        return false;
    }
}

void CollettData::closeProject() {
    m_project.reset(nullptr);
}

/**
 * Class Getters
 * =============
 */

bool CollettData::hasProject() const {
    if (m_project.isNull()) {
        return false;
    } else {
        return m_project.data()->isValid();
    }
}

Project *CollettData::project() {
    if (hasProject()) {
        return m_project.data();
    } else {
        return nullptr;
    }
}

StoryModel *CollettData::storyModel() {
    if (hasProject()) {
        return m_project.data()->storyModel();
    } else {
        return nullptr;
    }
}

} // namespace Collett
