/*
** Collett – Project Tree Class
** ============================
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

#include "tree.h"

#include <QJsonObject>
#include <QString>

using namespace Qt::Literals::StringLiterals;

namespace Collett {

// Constructor/Destructor
// ======================

Tree::Tree() {
}

Tree::~Tree() {
    qDebug() << "Destructor: Tree";
}

// Public Methods
// ==============

void Tree::pack(QJsonObject &data) {

    data["c:format"_L1] = "CollettProjectStructure";

}

void Tree::unpack(const QJsonObject &data) {

}

} // namespace Collett
