/*
** Collett - Scene Editor
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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import Collett

// One document in the editor stack, with its title above the text. The title
// is part of the project structure, not of the text. The arrow keys move the
// cursor between the title, the text and the neighbouring documents, so the
// stack reads as one text. The view gives focus to the root of its current
// item, so the root is a focus scope that passes it on to the text. The view
// also sets the x position of its items, so the item spans the view and
// centres the text.
FocusScope {
    id: root

    required property string handle
    required property int index
    required property int level
    required property string title
    required property int number
    required property bool hardBreak
    required property bool numbered
    required property int chapterNumber

    property Project project
    property ListView view
    property real textWidth: width

    readonly property real textTop: divider.height
    readonly property bool titleShown: title !== "" || titleInput.activeFocus
    readonly property bool bodyShown: textEdit.length > 0 || textEdit.activeFocus
    readonly property real textX: Math.max((width - textWidth) / 2, 0)

    // The formatting of the text, for the tool bar, which is only used while
    // the cursor is in the text
    readonly property DocumentBinder textBinder: binder
    readonly property bool textActive: textEdit.activeFocus

    signal sceneFocused(string handle)
    signal cursorMoved(rect rect)
    signal splitRequested(int position)
    signal upgradeRequested()
    signal deleteRequested(string handle, string name)

    implicitHeight: bodyBox.y + bodyBox.height

    // The view keeps its current item alive when it is scrolled out of view,
    // so the focused document keeps its cursor and selection
    onActiveFocusChanged: {
        if (!activeFocus) return;
        if (view) view.currentIndex = index;
        sceneFocused(handle);
    }

    // Entry Points
    // Used by the neighbouring documents and the main window to move the
    // cursor into this document.

    function enterAt(position: int) {
        textEdit.cursorPosition = position;
        textEdit.forceActiveFocus();
    }

    function enterTitleAt(position: int) {
        titleInput.cursorPosition = position;
        titleInput.forceActiveFocus();
    }

    function enterTextFromAbove(x: real) {
        const first = textEdit.positionToRectangle(0);
        enterAt(textEdit.positionAt(x, first.y + first.height / 2));
    }

    // Entering from above goes to the title, even when it is empty, so a
    // title can be added
    function enterFromAbove(x: real) {
        enterTitleAt(titleInput.positionAt(x, 0));
    }

    function enterFromBelow(x: real) {
        const last = textEdit.positionToRectangle(textEdit.length);
        enterAt(textEdit.positionAt(x, last.y + last.height / 2));
    }

    // Where the cursor goes when the document is opened from the project
    // view: the start of the text, or the end of the title if there is no
    // text but there is a title
    function enterStart() {
        if (textEdit.length === 0 && title !== "") {
            enterTitleAt(titleInput.length);
        } else {
            enterAt(0);
        }
    }

    // Merge this document into the previous one, and put the cursor where
    // the merged text starts. An empty first document has nothing to merge
    // into, so it is removed instead, and the cursor moves to the next one.
    function mergeUp(): bool {
        const previous = neighbour(-1);
        if (previous) {
            previous.enterAtEnd();
            const position = project.mergeDocument(handle);
            if (position >= 0) previous.enterAt(position);
            return true;
        }
        const next = neighbour(1);
        if (next && titleInput.text === "" && textEdit.length === 0) {
            next.enterStart();
            project.deleteDocument(handle);
            return true;
        }
        return false;
    }

    // Backspace at the start of the title merges the document into the
    // previous one, but only once the title is empty, so a title is never
    // lost in a merge
    function titleBackspace(event: KeyEvent) {
        if (event.key !== Qt.Key_Backspace || event.modifiers !== Qt.NoModifier) return;
        if (titleInput.cursorPosition !== 0 || titleInput.selectionStart !== titleInput.selectionEnd) return;
        if (titleInput.text === "") event.accepted = mergeUp();
    }

    // Backspace at the start of an indented paragraph removes the indent.
    // Backspace at the start of the text moves the cursor to the end of the
    // title if there is one, and otherwise merges the document into the
    // previous one. A run of Backspace presses from the text therefore clears
    // the title, and then merges.
    function textBackspace(event: KeyEvent) {
        if (event.key !== Qt.Key_Backspace || event.modifiers !== Qt.NoModifier) return;
        if (binder.removeFirstLineIndent()) {
            event.accepted = true;
            return;
        }
        if (textEdit.cursorPosition !== 0 || textEdit.selectionStart !== textEdit.selectionEnd) return;
        if (titleInput.text !== "") {
            enterTitleAt(titleInput.length);
            event.accepted = true;
        } else {
            event.accepted = mergeUp();
        }
    }

    function enterAtEnd() {
        enterAt(textEdit.length);
    }

    // The neighbouring document's editor. Editors are only created near the
    // visible part of the view, so the view is moved to create it if needed.
    function neighbour(offset: int): SceneEditor {
        const target = index + offset;
        if (!view || target < 0 || target >= view.count) return null;
        if (!view.itemAtIndex(target)) view.positionViewAtIndex(target, ListView.Contain);
        return view.itemAtIndex(target) as SceneEditor;
    }

    // The height of an empty paragraph in the document: one line at the
    // document's line spacing, plus the paragraph margins, which add up to
    // one font size
    FontMetrics {
        id: textMetrics

        font: binder.textFont
    }

    // The space above the document. Partitions and chapters get the space of
    // three empty paragraphs, and other documents the space of one. A scene
    // with a hard break before it gets a centred "* * *" with an empty
    // paragraph above and below it.
    Item {
        id: divider

        readonly property real paragraph: textMetrics.height * 1.15 + binder.textFont.pointSize
        readonly property bool major: root.level === Collett.PartitionLevel || root.level === Collett.ChapterLevel
        readonly property bool showBreak: root.index > 0 && root.hardBreak

        x: root.textX
        width: root.textWidth
        height: root.index === 0 ? 0 : (major ? 3 : showBreak ? 3 : 1) * paragraph

        Text {
            anchors.centerIn: parent
            text: "* * *"
            font: binder.textFont
            color: root.palette.text
            visible: divider.showBreak
        }
    }

    // The type of the document, above the title. Clicking it opens a menu
    // to change the type.
    Text {
        id: typeLabel

        objectName: "typeLabel"

        x: root.textX
        y: divider.height
        text: Labels.levelName(root.level)
        font: binder.headingFont
        color: Theme.levelColor(root.level)

        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }
        TapHandler {
            onTapped: typeMenu.popup(typeLabel, 0, typeLabel.height)
        }
    }

    // The number of the document in the margin, if it has one. It lines up
    // with the title, or with the first line of the text when the title is
    // hidden.
    Text {
        id: marker

        x: root.textX - width - 16
        y: root.titleShown ? titleBox.y : bodyBox.y + (textMetrics.height * 1.15 - height) / 2
        text: Labels.levelNumber(root.level, root.numbered, root.number, root.chapterNumber)
        font: titleInput.font
        color: Theme.levelColor(root.level)
        horizontalAlignment: Text.AlignRight
    }

    Menu {
        id: typeMenu

        objectName: "typeMenu"

        MenuItem {
            text: Labels.levelName(Collett.PartitionLevel)
            checkable: true
            checked: root.level === Collett.PartitionLevel
            onTriggered: root.project.model.setLevel(root.index, Collett.PartitionLevel)
        }
        MenuItem {
            text: Labels.levelName(Collett.ChapterLevel)
            checkable: true
            checked: root.level === Collett.ChapterLevel
            onTriggered: root.project.model.setLevel(root.index, Collett.ChapterLevel)
        }
        MenuItem {
            text: Labels.levelName(Collett.SceneLevel)
            checkable: true
            checked: root.level === Collett.SceneLevel
            onTriggered: root.project.model.setLevel(root.index, Collett.SceneLevel)
        }
        MenuItem {
            text: Labels.levelName(Collett.PageLevel)
            checkable: true
            checked: root.level === Collett.PageLevel
            onTriggered: root.project.model.setLevel(root.index, Collett.PageLevel)
        }
        MenuSeparator {}
        MenuItem {
            text: qsTr("Numbered")
            checkable: true
            checked: root.numbered
            enabled: root.level === Collett.ChapterLevel
            onTriggered: root.project.model.setNumbered(root.index, !root.numbered)
        }
        MenuItem {
            text: qsTr("Hard break")
            checkable: true
            checked: root.hardBreak
            enabled: root.level === Collett.SceneLevel
            onTriggered: root.project.model.setHardBreak(root.index, !root.hardBreak)
        }
        MenuSeparator {}
        MenuItem {
            objectName: "deleteDocumentItem"
            text: qsTr("Delete Document…")
            enabled: root.view ? root.view.count > 1 : false
            onTriggered: root.deleteRequested(root.handle, Labels.itemName(root.title, root.level, root.numbered, root.number, root.chapterNumber))
        }
    }

    // The title collapses when it is empty, unless the cursor is in it. It is
    // never hidden, as a hidden item cannot take focus.
    Item {
        id: titleBox

        x: root.textX
        y: typeLabel.y + typeLabel.height + 4
        width: root.textWidth
        height: root.titleShown ? titleInput.implicitHeight + 12 : 0
        clip: true

        TextInput {
            id: titleInput

            objectName: "titleInput"

            width: parent.width
            text: root.title
            wrapMode: TextInput.Wrap
            selectByMouse: true
            color: root.palette.text
            selectionColor: root.palette.highlight
            selectedTextColor: root.palette.highlightedText
            font: Fonts.scaled(binder.headingFont, root.level === Collett.PartitionLevel ? 2.0 : root.level === Collett.ChapterLevel ? 1.7 : 1.4)

            readonly property bool plainMove: selectionStart === selectionEnd

            onTextEdited: root.project.model.setTitle(root.index, text)
            onCursorRectangleChanged: {
                if (activeFocus) root.cursorMoved(mapToItem(root, cursorRectangle));
            }

            Text {
                text: qsTr("%1 title").arg(Labels.levelName(root.level))
                font: titleInput.font
                color: titleInput.color
                opacity: 0.4
                visible: titleInput.text === ""
            }

            Keys.onUpPressed: event => {
                const target = root.neighbour(-1);
                if (target && event.modifiers === Qt.NoModifier) {
                    target.enterFromBelow(cursorRectangle.x);
                } else {
                    event.accepted = false;
                }
            }
            Keys.onDownPressed: event => {
                if (event.modifiers === Qt.NoModifier) {
                    root.enterTextFromAbove(cursorRectangle.x);
                } else {
                    event.accepted = false;
                }
            }
            Keys.onLeftPressed: event => {
                const target = root.neighbour(-1);
                if (target && cursorPosition === 0 && plainMove && event.modifiers === Qt.NoModifier) {
                    target.enterAtEnd();
                } else {
                    event.accepted = false;
                }
            }
            Keys.onRightPressed: event => {
                if (cursorPosition === length && plainMove && event.modifiers === Qt.NoModifier) {
                    root.enterAt(0);
                } else {
                    event.accepted = false;
                }
            }
            Keys.onPressed: event => root.titleBackspace(event)
            // Ctrl+Enter changes the type of a newly split document
            function handleEnter(event: KeyEvent) {
                if (event.modifiers === Qt.ControlModifier) {
                    root.upgradeRequested();
                } else {
                    root.enterAt(0);
                }
            }

            Keys.onReturnPressed: event => handleEnter(event)
            Keys.onEnterPressed: event => handleEnter(event)
        }
    }

    // The text collapses when it is empty, unless the cursor is in it, the
    // same way as the title
    Item {
        id: bodyBox

        x: root.textX
        y: titleBox.y + titleBox.height
        width: root.textWidth
        height: root.bodyShown ? textEdit.height : 0
        clip: true

        TextEdit {
            id: textEdit

            objectName: "textEdit"

            width: parent.width
            focus: true
            wrapMode: TextEdit.Wrap
            selectByMouse: true
            persistentSelection: true
            color: root.palette.text
            selectionColor: root.palette.highlight
            selectedTextColor: root.palette.highlightedText

            readonly property bool plainMove: selectionStart === selectionEnd

            onCursorRectangleChanged: {
                if (activeFocus) root.cursorMoved(mapToItem(root, cursorRectangle));
            }

            // Ctrl+Enter splits the document at the cursor. The binder decides
            // the format of a paragraph started with Enter.
            function handleEnter(event: KeyEvent) {
                if (event.modifiers === Qt.ControlModifier && plainMove) {
                    root.splitRequested(cursorPosition);
                } else if (event.modifiers !== Qt.NoModifier || !binder.newParagraph()) {
                    event.accepted = false;
                }
            }

            Keys.onPressed: event => root.textBackspace(event)
            Keys.onReturnPressed: event => handleEnter(event)

            // Tab at the start of a paragraph adds a first-line indent, and
            // elsewhere inserts a tab
            Keys.onTabPressed: event => {
                if (event.modifiers !== Qt.NoModifier || !binder.addFirstLineIndent()) event.accepted = false;
            }
            Keys.onEnterPressed: event => handleEnter(event)

            // Each handler starts out accepted, so a key that stays within the
            // document must be passed back to the TextEdit
            Keys.onUpPressed: event => {
                const onFirstLine = cursorRectangle.y <= positionToRectangle(0).y + 1;
                if (onFirstLine && plainMove && event.modifiers === Qt.NoModifier) {
                    root.enterTitleAt(titleInput.positionAt(cursorRectangle.x, 0));
                } else {
                    event.accepted = false;
                }
            }
            Keys.onDownPressed: event => {
                const target = root.neighbour(1);
                const onLastLine = cursorRectangle.y >= positionToRectangle(length).y - 1;
                if (target && onLastLine && plainMove && event.modifiers === Qt.NoModifier) {
                    target.enterFromAbove(cursorRectangle.x);
                } else {
                    event.accepted = false;
                }
            }
            Keys.onLeftPressed: event => {
                if (cursorPosition === 0 && plainMove && event.modifiers === Qt.NoModifier) {
                    root.enterTitleAt(titleInput.length);
                } else {
                    event.accepted = false;
                }
            }
            Keys.onRightPressed: event => {
                const target = root.neighbour(1);
                if (target && cursorPosition === length && plainMove && event.modifiers === Qt.NoModifier) {
                    target.enterTitleAt(0);
                } else {
                    event.accepted = false;
                }
            }

            // A right click on a misspelled word offers spelling suggestions
            TapHandler {
                acceptedButtons: Qt.RightButton
                onTapped: eventPoint => spellMenu.openAt(eventPoint.position)
            }

            Text {
                text: qsTr("Body text")
                font: binder.textFont
                color: textEdit.color
                opacity: 0.4
                visible: textEdit.length === 0
            }
        }
    }

    // Spelling suggestions for a misspelled word, and adding it to the
    // project's dictionary
    Menu {
        id: spellMenu

        objectName: "spellMenu"

        property var spelling: ({})
        readonly property var suggestions: (spelling.suggestions ?? []).slice(0, 10)

        function openAt(position: point) {
            const found = binder.misspelledWordAt(textEdit.positionAt(position.x, position.y));
            if (!found.word) return;
            spelling = found;
            popup(textEdit, position);
        }

        Instantiator {
            model: spellMenu.suggestions
            delegate: MenuItem {
                required property string modelData

                text: modelData
                onTriggered: binder.replaceText(spellMenu.spelling.start, spellMenu.spelling.end, modelData)
            }
            onObjectAdded: (index, object) => spellMenu.insertItem(index, object as MenuItem)
            onObjectRemoved: (index, object) => spellMenu.removeItem(object as MenuItem)
        }
        MenuItem {
            text: qsTr("No Suggestions")
            enabled: false
            visible: spellMenu.suggestions.length === 0
            height: visible ? implicitHeight : 0
        }
        MenuSeparator {}
        MenuItem {
            text: qsTr("Add to Dictionary")
            onTriggered: binder.addWord(spellMenu.spelling.word)
        }
    }

    DocumentBinder {
        id: binder

        target: textEdit
        project: root.project
        handle: root.handle
        spellErrorColor: Theme.spellErrorColor
        formatErrorColor: Theme.formatErrorColor
    }
}
