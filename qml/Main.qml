/*
** Collett - Main Window
** =====================
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

ApplicationWindow {
    id: window

    width: 1400
    height: 900
    visible: true
    title: "Collett"

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Manuscript list: partitions, chapters and scenes in reading order
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 280
            color: window.palette.alternateBase

            Label {
                anchors.centerIn: parent
                text: "Project"
                opacity: 0.5
            }
        }

        // Editor: the stacked scene documents
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: window.palette.base

            Label {
                anchors.centerIn: parent
                text: "Editor"
                opacity: 0.5
            }
        }

        // Info column: meta data that follows the text as it scrolls
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 280
            color: window.palette.alternateBase

            Label {
                anchors.centerIn: parent
                text: "Info"
                opacity: 0.5
            }
        }
    }
}
