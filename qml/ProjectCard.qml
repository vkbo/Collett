/*
** Collett - Project Card
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

import Collett

// A document in the project list. Cards hidden by a folded partition or
// chapter collapse to zero height, so the cards below slide up. A card being
// dragged also collapses, but stays visible so it keeps the pointer grab.
Item {
    id: root

    required property int index
    required property string handle
    required property string title
    required property int level
    required property int words
    required property bool expanded
    required property bool foldable
    required property bool hidden

    property bool selected: false
    property bool dragged: false
    property real dropGap: 0

    signal openRequested(string handle)
    signal foldRequested(int index)
    signal dragStarted(int index)
    signal dragMoved(point scenePosition)
    signal dragFinished()

    readonly property color levelColor: Theme.levelColor(level)
    readonly property real gap: 6
    readonly property real cardHeight: card.height

    implicitHeight: hidden || dragged ? 0 : dropGap + card.height + gap
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

    Rectangle {
        id: card

        y: root.dropGap
        width: root.width
        height: content.implicitHeight + 16
        radius: 6
        color: Qt.rgba(root.levelColor.r, root.levelColor.g, root.levelColor.b, root.selected ? 0.28 : 0.12)
        border.width: root.selected ? 2 : 1
        border.color: root.levelColor

        TapHandler {
            onTapped: root.openRequested(root.handle)
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

        RowLayout {
            id: content

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 10
            anchors.rightMargin: 4
            spacing: 4

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Label {
                    Layout.fillWidth: true
                    text: root.title || Theme.levelName(root.level)
                    elide: Text.ElideRight
                    font.weight: root.level === Collett.PartitionLevel || root.level === Collett.ChapterLevel ? Font.DemiBold : Font.Normal
                }
                Label {
                    Layout.fillWidth: true
                    text: qsTr("%1 words").arg(root.words.toLocaleString(Qt.locale(), "f", 0))
                    elide: Text.ElideRight
                    font.pointSize: Application.font.pointSize * 0.85
                    opacity: 0.7
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
