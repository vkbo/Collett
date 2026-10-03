/*
** Collett - Settings Section
** ==========================
**
** This file is a part of Collett
** Copyright (C) 2026 Veronica Berglyd Olsen
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

import QtQuick
import QtQuick.Controls

// A titled part of the preferences, with its groups of settings. The
// sections follow each other in one scrolling page.
Control {
    id: root

    property string title
    default property alias groups: groupColumn.data

    width: parent ? parent.width : 0
    padding: 0
    bottomPadding: 32

    contentItem: Column {
        spacing: 12

        Label {
            text: root.title
            font.pointSize: Application.font.pointSize * 1.25
            font.weight: Font.DemiBold
        }

        Column {
            id: groupColumn

            width: parent.width
            spacing: 20
        }
    }
}
