/*
** Collett - Project Item
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
import QtQuick.Controls.impl
import QtQuick.Controls.Material
import QtQuick.Layouts

import Collett

// A document in the project list. Items hidden by a folded partition or
// chapter collapse to zero height, so the items below slide up. An item being
// dragged also collapses, but stays visible so it keeps the pointer grab.
Item {
    id: root

    required property int index
    required property string handle
    required property string title
    required property int level
    required property int words
    required property int number
    required property bool numbered
    required property int chapterNumber
    required property bool expanded
    required property bool foldable
    required property bool hidden

    property bool selected: false
    property bool dragged: false
    property real dropGap: 0

    // Deleting the last document of a project is not allowed
    property bool deletable: true

    readonly property string name: Labels.itemName(title, level, numbered, number, chapterNumber)

    signal openRequested(string handle)
    signal deleteRequested(string handle, string name)
    signal foldRequested(int index)
    signal dragStarted(int index)
    signal dragMoved(point scenePosition)
    signal dragFinished()

    readonly property color levelColor: Theme.levelColor(level)
    readonly property real gap: 2
    readonly property real itemHeight: entry.height

    implicitHeight: hidden || dragged ? 0 : dropGap + entry.height + gap
    opacity: hidden || dragged ? 0 : 1
    visible: implicitHeight > 0 || dragged
    clip: true

    Behavior on dropGap {
        NumberAnimation {
            duration: 180
            easing.type: Easing.OutCubic
        }
    }

    Behavior on implicitHeight {
        NumberAnimation {
            duration: 180
            easing.type: Easing.OutCubic
        }
    }
    Behavior on opacity {
        NumberAnimation {
            duration: 180
        }
    }

    // A standard list row: the level icon, the title with the word count
    // below, and the fold button at the end
    ItemDelegate {
        id: entry

        y: root.dropGap
        width: root.width
        leftPadding: 12
        rightPadding: 4
        topPadding: 6
        bottomPadding: 6
        highlighted: root.selected
        onClicked: root.openRequested(root.handle)

        // A right click opens the menu of actions on the document
        TapHandler {
            acceptedButtons: Qt.RightButton
            onTapped: eventPoint => itemMenu.popup(entry, eventPoint.position)
        }

        Menu {
            id: itemMenu

            objectName: "itemMenu"

            MenuItem {
                objectName: "deleteItem"
                text: qsTr("Delete Document…")
                enabled: root.deletable
                onTriggered: root.deleteRequested(root.handle, root.name)
            }
        }

        DragHandler {
            target: null
            xAxis.enabled: false
            grabPermissions: PointerHandler.CanTakeOverFromAnything
            onActiveChanged: active ? root.dragStarted(root.index) : root.dragFinished()
            onCentroidChanged: {
                if (active) root.dragMoved(centroid.scenePosition);
            }
        }

        contentItem: RowLayout {
            spacing: 12

            IconImage {
                Layout.preferredWidth: 20
                Layout.preferredHeight: 20
                source: "image://icons/" + Theme.levelIcon(root.level)
                sourceSize: Qt.size(20, 20)
                color: root.levelColor
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                Label {
                    Layout.fillWidth: true
                    text: root.name
                    elide: Text.ElideRight
                    font.weight: root.level === Collett.PartitionLevel || root.level === Collett.ChapterLevel ? Font.DemiBold : Font.Normal
                }
                Label {
                    Layout.fillWidth: true
                    text: Labels.wordCount(root.words)
                    elide: Text.ElideRight
                    font.pointSize: Application.font.pointSize * 0.85
                    color: Material.secondaryTextColor
                }
            }

            ToolButton {
                visible: root.foldable
                icon.source: "image://icons/arrow_right"
                icon.color: root.palette.windowText
                icon.width: 16
                icon.height: 16
                rotation: root.expanded ? 90 : 0
                onClicked: root.foldRequested(root.index)

                Behavior on rotation {
                    NumberAnimation {
                        duration: 150
                        easing.type: Easing.OutCubic
                    }
                }
            }
        }
    }
}
