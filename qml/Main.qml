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
import QtQuick.Controls.Material
import QtQuick.Layouts

import Collett

ApplicationWindow {
    id: window

    required property Project project

    // The document with the cursor
    property string focusHandle: ""

    width: 1400
    height: 900
    visible: true
    title: project.name ? project.name + " – Collett" : "Collett"
    Material.theme: Theme.materialTheme
    // Material colours its own controls, but leaves the palette alone. The
    // palette is set from the Material colours, so the parts drawn here follow
    // the theme too.
    palette.window: Material.dialogColor
    palette.windowText: Material.foreground
    palette.base: Material.background
    palette.text: Material.foreground
    palette.buttonText: Material.foreground
    palette.highlight: Material.textSelectionColor
    palette.highlightedText: Material.foreground
    palette.placeholderText: Material.hintTextColor

    // The document to show once the editor has a size. The split view sizes
    // the editor after the window is loaded, and an editor without a size
    // cannot scroll to a document.
    property string pendingScene: ""

    function showWhenReady(handle: string) {
        pendingScene = handle;
        Qt.callLater(showPending);
    }

    function showPending() {
        if (!pendingScene || editorView.height <= 0) return;
        editorView.showScene(pendingScene);
        pendingScene = "";
    }

    Component.onCompleted: {
        if (project.lastEditedHandle) showWhenReady(project.lastEditedHandle);
    }

    // A newly created project opens at its title page
    Connections {
        target: window.project

        function onProjectChanged() {
            if (window.project.lastEditedHandle) window.showWhenReady(window.project.lastEditedHandle);
        }
    }

    // Without a project, the window shows the form for creating one
    NewProject {
        anchors.fill: parent
        project: window.project
        visible: !window.project.isValid
    }

    // The document made by the last split, while the cursor stays in it
    property string splitHandle: ""

    // Split a document and put the cursor in the new title. The view must
    // lay itself out first, or it still has the old editor at the new row.
    function splitDocument(handle: string, position: int) {
        const newHandle = project.splitDocument(handle, position);
        if (!newHandle) return;
        Qt.callLater(() => {
            const newRow = project.model.rowOf(newHandle);
            editorView.forceLayout();
            if (!editorView.itemAtIndex(newRow)) editorView.positionViewAtIndex(newRow, ListView.Contain);
            const scene = editorView.itemAtIndex(newRow) as SceneEditor;
            if (scene) scene.enterTitleAt(0);
            splitHandle = newHandle;
        });
    }

    // Cycle a newly split document from scene, to scene with a hard break,
    // to chapter, and back to scene
    function upgradeDocument(handle: string) {
        if (handle !== splitHandle) return;
        const row = project.model.rowOf(handle);
        const scene = editorView.itemAtIndex(row) as SceneEditor;
        if (!scene) return;
        if (scene.level === Collett.ChapterLevel) {
            project.model.setLevel(row, Collett.SceneLevel);
        } else if (scene.hardBreak) {
            project.model.setHardBreak(row, false);
            project.model.setLevel(row, Collett.ChapterLevel);
        } else {
            project.model.setHardBreak(row, true);
        }
    }

    PreferencesDialog {
        id: preferences
    }

    Shortcut {
        sequences: ["Ctrl+,"]
        onActivated: preferences.openDialog()
    }

    Binding {
        target: Theme
        property: "dark"
        value: window.Material.theme === Material.Dark
    }

    // The side column can be resized, and its width is kept when the handle
    // is released
    SplitView {
        objectName: "mainSplit"

        anchors.fill: parent
        visible: window.project.isValid
        onResizingChanged: {
            if (resizing) return;
            Settings.sideBarWidth = sideBar.width;
            Settings.flushSettings();
        }

        // Manuscript list: partitions, chapters and scenes in reading order
        Rectangle {
            id: sideBar

            objectName: "sideBar"

            SplitView.preferredWidth: Settings.sideBarWidth
            SplitView.minimumWidth: 180
            SplitView.maximumWidth: window.width / 2
            color: window.palette.window

            ListView {
                id: projectList

                objectName: "projectList"

                // Dragging an item: the rows that move, the row they will be
                // put before, and the height of the gap that opens there
                property int dragRow: -1
                property int dragCount: 0
                property int dropRow: -1
                property real dropGap: 0
                property real pointerY: 0
                readonly property bool reordering: dragRow >= 0

                function startDrag(item: ProjectItem) {
                    dragRow = item.index;
                    dragCount = window.project.model.blockSize(item.index);
                    dropRow = dragRow + dragCount;
                    dropGap = item.itemHeight + item.gap;
                    dragProxy.title = item.title;
                    dragProxy.level = item.level;
                    dragProxy.words = item.words;
                    dragProxy.number = item.number;
                    dragProxy.numbered = item.numbered;
                    dragProxy.chapterNumber = item.chapterNumber;
                    dragProxy.expanded = item.expanded;
                    dragProxy.foldable = item.foldable;
                }

                function moveDrag(scenePosition: point) {
                    pointerY = mapFromItem(null, scenePosition).y;
                    updateDrop();
                }

                // The drop goes before the item under the pointer if it is in
                // the upper half of it, or else after it and any rows it
                // hides. Away from the items, it goes first or last.
                function updateDrop() {
                    const y = pointerY + contentY;
                    const row = indexAt(width / 2, y);
                    let target = -1;
                    if (row < 0) {
                        target = y < originY ? 0 : count;
                    } else if (row < dragRow || row >= dragRow + dragCount) {
                        const item = itemAtIndex(row) as ProjectItem;
                        const middle = (dropRow === row ? dropGap : 0) + item.itemHeight / 2;
                        target = y - item.y < middle ? row : row + window.project.model.blockSize(row);
                    }
                    if (target === dragRow) target = dragRow + dragCount;
                    if (target >= 0) dropRow = target;
                }

                function finishDrag() {
                    if (!reordering) return;
                    if (dropRow !== dragRow + dragCount) window.project.model.moveBlock(dragRow, dragCount, dropRow);
                    dragRow = -1;
                    dropRow = -1;
                }

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: sideBarFooter.top
                anchors.margins: 8
                clip: true
                model: window.project.model
                boundsBehavior: Flickable.StopAtBounds
                interactive: !reordering

                // Mouse drags move items, so only the wheel, touchpad and
                // touch scroll the list
                acceptedButtons: Qt.NoButton
                ScrollBar.vertical: ScrollBar {}

                // The dragged item must not be destroyed while it holds the
                // pointer, so all items are kept while dragging
                cacheBuffer: reordering ? 100000 : 0

                delegate: ProjectItem {
                    id: projectItem

                    width: projectList.width
                    selected: projectItem.handle === window.focusHandle
                    dragged: projectList.dragRow === index
                    dropGap: projectList.dropRow === index ? projectList.dropGap : 0
                    onOpenRequested: handle => editorView.showScene(handle)
                    onFoldRequested: index => window.project.model.toggleExpanded(index)
                    onDragStarted: projectList.startDrag(projectItem)
                    onDragMoved: scenePosition => projectList.moveDrag(scenePosition)
                    onDragFinished: projectList.finishDrag()
                }

                footer: Item {
                    width: projectList.width
                    height: projectList.dropRow === projectList.count ? projectList.dropGap : 0

                    Behavior on height {
                        NumberAnimation {
                            duration: 180
                            easing.type: Easing.OutCubic
                        }
                    }
                }

                // Scroll while the pointer is near the top or bottom edge
                Timer {
                    readonly property real edge: 32
                    readonly property real step: projectList.pointerY < edge ? -8 : 8

                    interval: 16
                    repeat: true
                    running: projectList.reordering && (projectList.pointerY < edge || projectList.pointerY > projectList.height - edge)
                    onTriggered: {
                        const top = projectList.originY;
                        const bottom = projectList.originY + projectList.contentHeight - projectList.height;
                        projectList.contentY = Math.max(top, Math.min(projectList.contentY + step, bottom));
                        projectList.updateDrop();
                    }
                }
            }

            RowLayout {
                id: sideBarFooter

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 8

                // The buttons sit at the right, with the settings at the edge
                Item {
                    Layout.fillWidth: true
                }

                // Cycles between following the system, light and dark
                ToolButton {
                    objectName: "themeButton"

                    readonly property list<string> icons: ["theme_auto", "theme_light", "theme_dark"]
                    readonly property list<string> labels: [qsTr("Colour theme: Follow the system"), qsTr("Colour theme: Light"), qsTr("Colour theme: Dark")]

                    icon.source: "image://icons/" + icons[Settings.themeMode]
                    icon.color: palette.buttonText
                    ToolTip.text: labels[Settings.themeMode]
                    ToolTip.visible: hovered
                    ToolTip.delay: 500
                    Accessible.name: labels[Settings.themeMode]
                    onClicked: Theme.nextThemeMode()
                }

                ToolButton {
                    objectName: "preferencesButton"

                    icon.source: "image://icons/settings"
                    icon.color: palette.buttonText
                    ToolTip.text: qsTr("Preferences")
                    ToolTip.visible: hovered
                    ToolTip.delay: 500
                    Accessible.name: qsTr("Preferences")
                    onClicked: preferences.openDialog()
                }
            }

            // A copy of the dragged item that follows the pointer, lifted
            // above the list
            Pane {
                x: projectList.x
                y: projectList.y + projectList.pointerY - height / 2
                width: projectList.width
                padding: 0
                visible: projectList.reordering
                Material.elevation: 6

                ProjectItem {
                    id: dragProxy

                    index: -1
                    handle: ""
                    title: ""
                    level: 0
                    words: 0
                    number: 0
                    numbered: false
                    chapterNumber: 0
                    expanded: true
                    foldable: false
                    hidden: false
                    selected: true

                    width: parent.width
                }
            }
        }

        // Editor: every document of the group, stacked in reading order.
        // Editors are only created near the visible part of the view.
        Rectangle {
            SplitView.fillWidth: true
            SplitView.minimumWidth: 300
            color: window.palette.base

            EditorToolBar {
                id: editorToolBar

                objectName: "editorToolBar"

                readonly property SceneEditor scene: editorView.currentItem as SceneEditor

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                binder: scene?.textBinder ?? null
                active: scene?.textActive ?? false
            }

            ListView {
                id: editorView

                objectName: "editorView"

                onHeightChanged: window.showPending()

                // The side margin leaves room for the document markers to the
                // left of the text
                readonly property real margin: 48
                readonly property real sideMargin: 140
                readonly property real maxTextWidth: 720
                readonly property real textWidth: Math.min(width - 2 * sideMargin, maxTextWidth)

                // How far the view is scrolled, from 0 at the top to 1 at
                // the bottom. The content height is an estimate, as only the
                // editors near the view exist.
                readonly property real scrollRange: Math.max(contentHeight - height, 1)
                readonly property real scrollFraction: Math.min(Math.max((contentY - originY) / scrollRange, 0), 1)

                function scrollToFraction(fraction: real) {
                    contentY = originY + fraction * scrollRange;
                }

                anchors.top: editorToolBar.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                clip: true
                model: window.project.model
                cacheBuffer: Math.max(2 * height, 0)
                boundsBehavior: Flickable.StopAtBounds

                // The current item marks the focused document, so the view
                // must not scroll to it on its own, or change it on key
                // presses the text does not use
                highlightFollowsCurrentItem: false
                keyNavigationEnabled: false

                header: Item {
                    height: editorView.margin
                }
                footer: Item {
                    height: editorView.margin
                }

                // Scroll just enough to show a rectangle in content coordinates
                function ensureVisible(rect: rect) {
                    const bottom = rect.y + rect.height;
                    if (rect.y < contentY) {
                        contentY = rect.y;
                    } else if (bottom > contentY + height) {
                        contentY = bottom - height;
                    }
                }

                // Scroll a document to the top of the view and put the cursor
                // at its start
                function showScene(handle: string) {
                    const row = window.project.model.rowOf(handle);
                    if (row < 0) return;
                    positionViewAtIndex(row, ListView.Beginning);
                    const scene = itemAtIndex(row) as SceneEditor;
                    if (!scene) return;
                    contentY = Math.max(originY, scene.y + scene.textTop - margin);
                    returnToBounds();
                    scene.enterStart();
                }

                delegate: SceneEditor {
                    id: sceneEditor

                    width: editorView.width
                    textWidth: editorView.textWidth
                    project: window.project
                    view: editorView

                    onSplitRequested: position => window.splitDocument(sceneEditor.handle, position)
                    onUpgradeRequested: window.upgradeDocument(sceneEditor.handle)

                    onSceneFocused: handle => {
                        if (handle !== window.splitHandle) window.splitHandle = "";
                        window.focusHandle = handle;
                        window.project.setLastEditedHandle(handle);
                    }
                    onCursorMoved: rect => editorView.ensureVisible(sceneEditor.mapToItem(editorView.contentItem, rect))
                }
            }

            // A scroll bar with a fixed handle size. One attached to the view
            // would resize its handle as the estimated content height
            // changes. It follows the view, unless it is being dragged, and
            // then the view follows it.
            ScrollBar {
                id: editorScroll

                readonly property real travel: 1 - size

                anchors.top: editorView.top
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                orientation: Qt.Vertical
                size: 0.1
                active: hovered || pressed || editorView.moving

                onPositionChanged: {
                    if (pressed) editorView.scrollToFraction(position / travel);
                }

                Binding on position {
                    when: !editorScroll.pressed
                    value: editorView.scrollFraction * editorScroll.travel
                    restoreMode: Binding.RestoreNone
                }
            }
        }
    }
}
