/*
** Collett - Group Editor
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

// The documents of a group as one text, each with its title above its text.
// The text is one document, so the cursor, the selection and the undo
// history work across documents. Around each title, the editor draws the
// type of the document, its number in the margin, and a hard break.
FocusScope {
    id: root

    property Project project
    property real textWidth: width

    // The space above the first document and below the last
    property real margin: 48

    // The formatting of the text, for the tool bar
    readonly property DocumentBinder binder: textBinder
    readonly property bool textActive: textEdit.activeFocus

    readonly property real textX: Math.max((width - textWidth) / 2, 0)

    signal deleteRequested(string handle, string name)

    // Scroll a document's type label to the top of the view, and put the
    // cursor where the document starts
    function showItem(handle: string) {
        const rect = textBinder.titleRect(handle);
        if (rect.height <= 0) return;
        textBinder.enterItem(handle);
        textEdit.forceActiveFocus();
        const top = textEdit.y + rect.y - textBinder.labelHeight() - 4 - margin;
        flick.contentY = Math.max(0, Math.min(top, flick.contentHeight - flick.height));
    }

    // Scroll just enough to show a position in the text
    function showPosition(position: int) {
        flick.ensureVisible(textEdit.mapToItem(flick.contentItem, textEdit.positionToRectangle(position)));
    }

    // The title font for a level, as the document uses it
    function titleFont(level: int): font {
        return Fonts.scaled(textBinder.headingFont, level === Collett.PartitionLevel ? 2.0 : level === Collett.ChapterLevel ? 1.7 : 1.4);
    }

    Flickable {
        id: flick

        objectName: "editorFlick"

        anchors.fill: parent
        clip: true
        contentWidth: width
        contentHeight: Math.max(textEdit.y + textEdit.height + root.margin, height)
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar {}

        // Scroll just enough to show a rectangle in content coordinates
        function ensureVisible(rect: rect) {
            const bottom = rect.y + rect.height;
            if (rect.y < contentY) {
                contentY = Math.max(rect.y, 0);
            } else if (bottom > contentY + height) {
                contentY = bottom - height;
            }
        }

        // The space above the first title holds the divider from a document
        // before it, so it is moved out of view
        TextEdit {
            id: textEdit

            objectName: "textEdit"

            x: root.textX
            y: root.margin - textBinder.topSpace
            width: root.textWidth
            focus: true
            // An empty document is laid out with the font of the TextEdit, so
            // it must match the text font, or the cursor is cut off
            font: textBinder.textFont
            wrapMode: TextEdit.Wrap
            selectByMouse: true
            persistentSelection: true
            color: root.palette.text
            selectionColor: root.palette.highlight
            selectedTextColor: root.palette.highlightedText

            onCursorRectangleChanged: {
                if (activeFocus) flick.ensureVisible(mapToItem(flick.contentItem, cursorRectangle));
            }

            // The keys that work differently around titles are handled by the
            // textBinder. A key it does not handle is passed on to the TextEdit.
            Keys.onPressed: event => {
                const modifiers = event.modifiers & ~Qt.KeypadModifier;
                let handled = false;
                if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                    handled = textBinder.keyEnter(modifiers);
                } else if (modifiers === Qt.NoModifier && event.key === Qt.Key_Backspace) {
                    handled = textBinder.keyBackspace();
                } else if (modifiers === Qt.NoModifier && event.key === Qt.Key_Delete) {
                    handled = textBinder.keyDelete();
                } else if (modifiers === Qt.NoModifier && event.key === Qt.Key_Tab) {
                    handled = textBinder.keyTab();
                }
                event.accepted = handled;
            }

            // A right click on a misspelled word offers spelling suggestions
            TapHandler {
                acceptedButtons: Qt.RightButton
                onTapped: eventPoint => spellMenu.openAt(eventPoint.position)
            }
        }

        Repeater {
            id: decorations

            model: root.project ? root.project.model : null

            delegate: ItemDecoration {}
        }
    }

    // What is drawn around the title of a document. The positions follow the
    // layout of the text.
    component ItemDecoration: Item {
        id: decoration

        required property int index
        required property string handle
        required property string title
        required property int level
        required property int number
        required property bool hardBreak
        required property bool numbered
        required property int chapterNumber

        readonly property rect titleRect: textBinder.layoutRevision >= 0 ? textBinder.titleRect(handle) : Qt.rect(0, 0, 0, 0)
        readonly property rect bodyRect: textBinder.layoutRevision >= 0 ? textBinder.emptyBodyRect(handle) : Qt.rect(0, 0, 0, 0)
        readonly property real labelHeight: textBinder.headingFont.pointSize > 0 ? textBinder.labelHeight() : 0
        readonly property real dividerHeight: textBinder.textFont.pointSize > 0 ? textBinder.dividerHeight(level, hardBreak) : 0
        readonly property real titleTop: titleRect.y

        x: textEdit.x
        y: textEdit.y
        width: textEdit.width
        visible: titleRect.height > 0

        // A hard break before a scene, in the middle of the divider
        Text {
            width: parent.width
            y: decoration.titleTop - 4 - decoration.labelHeight - decoration.dividerHeight / 2 - height / 2
            horizontalAlignment: Text.AlignHCenter
            text: "* * *"
            font: textBinder.textFont
            color: root.palette.text
            visible: decoration.index > 0 && decoration.hardBreak && decoration.level === Collett.SceneLevel
        }

        // The type of the document, above the title. Clicking it opens a
        // menu to change the type.
        Text {
            id: typeLabel

            objectName: "typeLabel"

            y: decoration.titleTop - 4 - height
            text: Labels.levelName(decoration.level)
            font: textBinder.headingFont
            color: Theme.levelColor(decoration.level)

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
            TapHandler {
                onTapped: typeMenu.openFor(decoration, typeLabel)
            }
        }

        // The number of the document in the margin, if it has one
        Text {
            x: -width - 16
            y: decoration.titleTop
            text: Labels.levelNumber(decoration.level, decoration.numbered, decoration.number, decoration.chapterNumber)
            font: root.titleFont(decoration.level)
            color: Theme.levelColor(decoration.level)
            horizontalAlignment: Text.AlignRight
        }

        Text {
            x: decoration.titleRect.x
            y: decoration.titleTop
            text: qsTr("%1 title").arg(Labels.levelName(decoration.level))
            font: root.titleFont(decoration.level)
            color: textEdit.color
            opacity: 0.4
            visible: decoration.title === ""
        }

        Text {
            x: decoration.bodyRect.x
            y: decoration.bodyRect.y
            text: qsTr("Body text")
            font: textBinder.textFont
            color: textEdit.color
            opacity: 0.4
            visible: decoration.bodyRect.height > 0
        }
    }

    // The menu of a document's type label
    Menu {
        id: typeMenu

        objectName: "typeMenu"

        property int row: -1
        property string handle: ""
        property string itemTitle: ""
        property int level: 0
        property bool numbered: false
        property bool hardBreak: false
        property int number: 0
        property int chapterNumber: 0

        function openFor(decoration: ItemDecoration, label: Item) {
            row = decoration.index;
            handle = decoration.handle;
            itemTitle = decoration.title;
            level = decoration.level;
            numbered = decoration.numbered;
            hardBreak = decoration.hardBreak;
            number = decoration.number;
            chapterNumber = decoration.chapterNumber;
            popup(label, 0, label.height);
        }

        MenuItem {
            text: Labels.levelName(Collett.PartitionLevel)
            checkable: true
            checked: typeMenu.level === Collett.PartitionLevel
            onTriggered: root.project.model.setLevel(typeMenu.row, Collett.PartitionLevel)
        }
        MenuItem {
            text: Labels.levelName(Collett.ChapterLevel)
            checkable: true
            checked: typeMenu.level === Collett.ChapterLevel
            onTriggered: root.project.model.setLevel(typeMenu.row, Collett.ChapterLevel)
        }
        MenuItem {
            text: Labels.levelName(Collett.SceneLevel)
            checkable: true
            checked: typeMenu.level === Collett.SceneLevel
            onTriggered: root.project.model.setLevel(typeMenu.row, Collett.SceneLevel)
        }
        MenuItem {
            text: Labels.levelName(Collett.PageLevel)
            checkable: true
            checked: typeMenu.level === Collett.PageLevel
            onTriggered: root.project.model.setLevel(typeMenu.row, Collett.PageLevel)
        }
        MenuSeparator {}
        MenuItem {
            text: qsTr("Numbered")
            checkable: true
            checked: typeMenu.numbered
            enabled: typeMenu.level === Collett.ChapterLevel
            onTriggered: root.project.model.setNumbered(typeMenu.row, !typeMenu.numbered)
        }
        MenuItem {
            text: qsTr("Hard break")
            checkable: true
            checked: typeMenu.hardBreak
            enabled: typeMenu.level === Collett.SceneLevel
            onTriggered: root.project.model.setHardBreak(typeMenu.row, !typeMenu.hardBreak)
        }
        MenuSeparator {}
        MenuItem {
            objectName: "deleteDocumentItem"
            text: qsTr("Delete Document…")
            enabled: decorations.count > 1
            onTriggered: root.deleteRequested(typeMenu.handle, Labels.itemName(typeMenu.itemTitle, typeMenu.level, typeMenu.numbered, typeMenu.number, typeMenu.chapterNumber))
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
            const found = textBinder.misspelledWordAt(textEdit.positionAt(position.x, position.y));
            if (!found.word) return;
            spelling = found;
            popup(textEdit, position);
        }

        Instantiator {
            model: spellMenu.suggestions
            delegate: MenuItem {
                required property string modelData

                text: modelData
                onTriggered: textBinder.replaceText(spellMenu.spelling.start, spellMenu.spelling.end, modelData)
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
            onTriggered: textBinder.addWord(spellMenu.spelling.word)
        }
    }

    DocumentBinder {
        id: textBinder

        target: textEdit
        project: root.project
        spellErrorColor: Theme.spellErrorColor
        formatErrorColor: Theme.formatErrorColor
    }
}
