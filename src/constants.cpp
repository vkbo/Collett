/*
** Collett – Collett Constants
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

#include "constants.h"

#include <QCoreApplication>
#include <QString>

namespace Collett {

QString itemClassNames(ItemClass itemClass) {

    static const char *values[] = {
        QT_TRANSLATE_NOOP("ItemClass", "Novel"),
        QT_TRANSLATE_NOOP("ItemClass", "Characters"),
        QT_TRANSLATE_NOOP("ItemClass", "Plot"),
        QT_TRANSLATE_NOOP("ItemClass", "Locations"),
        QT_TRANSLATE_NOOP("ItemClass", "Objects"),
        QT_TRANSLATE_NOOP("ItemClass", "Entities"),
        QT_TRANSLATE_NOOP("ItemClass", "Custom"),
        QT_TRANSLATE_NOOP("ItemClass", "Archive"),
        QT_TRANSLATE_NOOP("ItemClass", "Trash"),
    };
    return QCoreApplication:: translate("ItemClass", values[itemClass]);
}

QString itemLevelNames(ItemLevel itemLevel) {

    static const char *values[] = {
        QT_TRANSLATE_NOOP("ItemLevel", "Novel Document"),
        QT_TRANSLATE_NOOP("ItemLevel", "Novel Partition"),
        QT_TRANSLATE_NOOP("ItemLevel", "Novel Chapter"),
        QT_TRANSLATE_NOOP("ItemLevel", "Novel Scene"),
        QT_TRANSLATE_NOOP("ItemLevel", "Project Note"),
    };
    return QCoreApplication:: translate("ItemLevel", values[itemLevel]);
}

} // namespace Collett
