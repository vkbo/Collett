/*
** Collett - Settings Group
** ========================
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

// A labelled frame of settings rows
Control {
    id: root

    property string title
    default property alias rows: rowColumn.data

    width: parent ? parent.width : 0
    padding: 0

    contentItem: Column {
        spacing: 4

        Label {
            text: root.title
            visible: text !== ""
            topPadding: 4
            bottomPadding: 4
            font.weight: Font.Medium
        }

        Frame {
            width: parent.width
            padding: 0

            Column {
                id: rowColumn

                width: parent.width
            }
        }
    }
}
