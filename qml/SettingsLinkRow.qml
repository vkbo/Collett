/*
** Collett - Settings Link Row
** ===========================
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

// A setting that opens a page of its own: its title and help text on the
// left, and its current value and a chevron on the right. The whole row is
// clickable.
ItemDelegate {
    id: root

    property string title
    property string help
    property string value

    width: parent ? parent.width : 0
    leftPadding: 16
    rightPadding: 8
    topPadding: 12
    bottomPadding: 12
    Accessible.name: title
    Accessible.description: help

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

        Label {
            text: root.value
            opacity: 0.7
        }

        // Only a marker, so clicks go to the row
        ToolButton {
            icon.source: "image://icons/arrow_right"
            icon.color: root.palette.windowText
            enabled: false
            opacity: 0.7
            focusPolicy: Qt.NoFocus
            Accessible.ignored: true
        }
    }
}
