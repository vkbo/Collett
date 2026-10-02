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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Collett

ApplicationWindow {
    id: window

    required property Project project

    // The handle of the document shown in the editor
    property string openHandle: project.lastEditedHandle

    width: 1400
    height: 900
    visible: true
    title: project.name ? project.name + " – Collett" : "Collett"

    Binding {
        target: Theme
        property: "dark"
        value: window.palette.window.hslLightness < 0.5
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Manuscript list: partitions, chapters and scenes in reading order
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 280
            color: window.palette.window

            ListView {
                id: projectList

                anchors.fill: parent
                anchors.margins: 8
                clip: true
                model: window.project.model
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ScrollBar {}

                delegate: ProjectCard {
                    id: projectCard

                    width: projectList.width
                    selected: projectCard.handle === window.openHandle
                    onOpenRequested: handle => window.openHandle = handle
                    onFoldRequested: index => window.project.model.toggleExpanded(index)
                }
            }
        }

        // Editor: the stacked scene documents
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: window.palette.base

            Flickable {
                id: editorView

                readonly property real margin: 48
                readonly property real maxTextWidth: 720

                anchors.fill: parent
                clip: true
                contentWidth: width
                contentHeight: textEdit.height + 2 * margin
                flickableDirection: Flickable.VerticalFlick
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ScrollBar {}

                // Scroll just enough to show a rectangle in text coordinates
                function ensureVisible(rect) {
                    const top = textEdit.y + rect.y;
                    const bottom = top + rect.height;
                    if (top < contentY) {
                        contentY = top;
                    } else if (bottom > contentY + height) {
                        contentY = bottom - height;
                    }
                }

                TextEdit {
                    id: textEdit

                    x: Math.max((editorView.width - width) / 2, 0)
                    y: editorView.margin
                    width: Math.min(editorView.width - 2 * editorView.margin, editorView.maxTextWidth)
                    focus: true
                    wrapMode: TextEdit.Wrap
                    selectByMouse: true
                    persistentSelection: true
                    color: window.palette.text
                    selectionColor: window.palette.highlight
                    selectedTextColor: window.palette.highlightedText

                    onCursorRectangleChanged: editorView.ensureVisible(cursorRectangle)
                }

                DocumentBinder {
                    target: textEdit
                    project: window.project
                    handle: window.openHandle
                }
            }
        }
    }
}
