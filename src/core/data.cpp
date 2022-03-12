/*
** Collett – Core Data Class
** =========================
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

#include "data.h"
#include "collection.h"

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
    m_collection.reset();
}

/**
 * Class Methods
 * =============
 */

void CollettData::newCollection() {
    if (!hasCollection()) {
        m_collection.reset(new Collection());
    }
}

void CollettData::openCollection(const QString &path) {

    m_collection.reset(new Collection());
    m_collection.data()->openCollection(path);
    if (m_collection.data()->hasError()) {
        m_collection.reset(nullptr);
    }
}

void CollettData::saveCollection() {
    if (hasCollection()) {
        return m_collection.data()->saveCollection();
    }
}

void CollettData::closeCollection() {
    m_collection.reset(nullptr);
}

/**
 * Class Getters
 * =============
 */

bool CollettData::hasCollection() const {
    return !m_collection.isNull();
}

Collection *CollettData::collection() {
    if (hasCollection()) {
        return m_collection.data();
    } else {
        return nullptr;
    }
}

} // namespace Collett
