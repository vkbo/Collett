/*
** Collett – Main Theme Class
** ==========================
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

#include "collett.h"
#include "theme.h"

#include <QString>

namespace Collett {

// Constructor/Destructor/Instance
// ===============================

Theme *Theme::staticInstance = nullptr;
Theme *Theme::instance() {
    if (staticInstance == nullptr) {
        staticInstance = new Theme();
        qDebug() << "Constructor: Theme";
    }
    return staticInstance;
}

void Theme::destroy() {
    if (staticInstance != nullptr) {
        qDebug() << "Destructor: Static Theme";
        delete Theme::staticInstance;
    }
}

Theme::Theme(QObject *parent) : QObject(parent) {
}

Theme::~Theme() {
    qDebug() << "Destructor: Theme";
}

// Public Methods
// ==============

void Theme::loadTheme(QString theme) {

}

} // namespace Collett
