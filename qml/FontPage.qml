/*
** Collett - Font Page
** ===================
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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Collett

// A page for picking a font family, style and size, with a preview. Each
// pick is reported with chosen() straight away.
Page {
    id: root

    required property font initialFont

    property string family: initialFont.family
    property string style: Fonts.styleOf(initialFont)
    property real size: initialFont.pointSize

    readonly property list<string> families: Qt.fontFamilies()
    readonly property font currentFont: Fonts.font(family, style, size)

    signal chosen(font selected)
    signal back

    objectName: "fontPage"
    padding: 0

    // A new family keeps the style if it has it, or falls back to the first
    function pickFamily(name: string) {
        const available = Fonts.styles(name);
        family = name;
        if (!available.includes(style))
            style = available.includes("Regular") ? "Regular" : available[0] ?? "";
        chosen(currentFont);
    }

    function pickStyle(name: string) {
        style = name;
        chosen(currentFont);
    }

    function pickSize(points: real) {
        size = points;
        chosen(currentFont);
    }

    Component.onCompleted: {
        familyList.positionViewAtIndex(familyList.model.indexOf(family), ListView.Center);
        search.forceActiveFocus();
    }

    header: RowLayout {
        spacing: 4

        ToolButton {
            objectName: "fontPageBack"

            icon.source: "image://icons/arrow_left"
            icon.color: palette.windowText
            Accessible.name: qsTr("Back")
            onClicked: root.back()
        }
        Label {
            Layout.fillWidth: true
            text: root.title
            font.pointSize: Application.font.pointSize * 1.25
            font.weight: Font.DemiBold
            elide: Text.ElideRight
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 8
        spacing: 12

        TextField {
            id: search

            objectName: "fontSearch"

            Layout.fillWidth: true
            placeholderText: qsTr("Search fonts")
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            // The family gets most of the width, and the style and size
            // share a column next to it
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 0
                Layout.horizontalStretchFactor: 2

                Label {
                    text: qsTr("Family")
                    font.weight: Font.Medium
                }
                Frame {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    padding: 0

                    ListView {
                        id: familyList

                        objectName: "fontFamilies"

                        anchors.fill: parent
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds
                        model: {
                            const text = search.text.trim().toLowerCase();
                            return text ? root.families.filter(name => name.toLowerCase().includes(text)) : root.families;
                        }
                        currentIndex: model.indexOf(root.family)

                        ScrollBar.vertical: ScrollBar {}

                        delegate: ItemDelegate {
                            required property string modelData

                            width: ListView.view.width
                            text: modelData
                            font.family: modelData
                            highlighted: modelData === root.family
                            onClicked: root.pickFamily(modelData)
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 0
                Layout.horizontalStretchFactor: 1

                Label {
                    text: qsTr("Style")
                    font.weight: Font.Medium
                }
                Frame {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    padding: 0

                    ListView {
                        objectName: "fontStyles"

                        anchors.fill: parent
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds
                        model: Fonts.styles(root.family)

                        ScrollBar.vertical: ScrollBar {}

                        delegate: ItemDelegate {
                            required property string modelData

                            width: ListView.view.width
                            text: modelData
                            font: Fonts.font(root.family, modelData, Application.font.pointSize)
                            highlighted: modelData === root.style
                            onClicked: root.pickStyle(modelData)
                        }
                    }
                }

                Label {
                    Layout.topMargin: 8
                    text: qsTr("Size")
                    font.weight: Font.Medium
                }
                SpinBox {
                    objectName: "fontSize"

                    Layout.fillWidth: true
                    from: 4
                    to: 200
                    editable: true
                    value: Math.round(root.size)
                    Accessible.name: qsTr("Size")
                    onValueModified: root.pickSize(value)
                }
                Frame {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    padding: 0

                    ListView {
                        anchors.fill: parent
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds
                        model: Fonts.sizes()

                        ScrollBar.vertical: ScrollBar {}

                        delegate: ItemDelegate {
                            required property int modelData

                            width: ListView.view.width
                            text: modelData
                            highlighted: modelData === Math.round(root.size)
                            onClicked: root.pickSize(modelData)
                        }
                    }
                }
            }
        }

        Label {
            text: qsTr("Preview")
            font.weight: Font.Medium
        }
        Frame {
            Layout.fillWidth: true

            Label {
                objectName: "fontPreview"

                width: parent.width
                text: qsTr("The quick brown fox jumps over the lazy dog.")
                font: root.currentFont
                elide: Text.ElideRight
            }
        }
    }
}
