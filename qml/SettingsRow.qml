/*
** Collett - Settings Row
** ======================
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
import QtQuick.Layouts

// A setting in a group: its title and help text on the left, and its
// controls on the right
Control {
    id: root

    property string title
    property string help
    default property alias controls: controlRow.data

    width: parent ? parent.width : 0
    leftPadding: 16
    rightPadding: 16
    topPadding: 12
    bottomPadding: 12

    contentItem: RowLayout {
        spacing: 16

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Label {
                Layout.fillWidth: true
                text: root.title
                font.weight: Font.DemiBold
                wrapMode: Text.Wrap
            }
            Label {
                Layout.fillWidth: true
                text: root.help
                visible: text !== ""
                wrapMode: Text.Wrap
                opacity: 0.7
            }
        }

        RowLayout {
            id: controlRow

            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            spacing: 4
        }
    }
}
