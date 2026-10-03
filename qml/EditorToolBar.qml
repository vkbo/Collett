/*
** Collett - Editor Tool Bar
** =========================
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

// Text formatting for the document that has the cursor. The buttons show the
// format at the cursor, and their shortcuts work while the cursor is in the
// text. The buttons never take focus, so the cursor stays in the text.
ToolBar {
    id: root

    // The binder of the document that has the cursor, if any
    property DocumentBinder binder: null
    property bool active: false

    readonly property bool ready: active && binder !== null

    background: Rectangle {
        implicitHeight: 40
        color: root.palette.base
    }

    component FormatButton: ToolButton {
        implicitWidth: 36
        implicitHeight: 36
        icon.width: 20
        icon.height: 20
        focusPolicy: Qt.NoFocus
        display: AbstractButton.IconOnly
        ToolTip.text: action ? action.text : ""
        ToolTip.visible: hovered
        ToolTip.delay: 500
    }

    component FormatAction: Action {
        enabled: root.ready
    }

    // The text is centred in the editor, so the buttons are too
    RowLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 0

        FormatButton {
            objectName: "boldButton"
            action: FormatAction {
                text: qsTr("Bold")
                icon.source: "image://icons/fmt_bold"
                shortcut: StandardKey.Bold
                checked: root.binder?.bold ?? false
                onTriggered: root.binder.bold = !root.binder.bold
            }
        }
        FormatButton {
            objectName: "italicButton"
            action: FormatAction {
                text: qsTr("Italic")
                icon.source: "image://icons/fmt_italic"
                shortcut: StandardKey.Italic
                checked: root.binder?.italic ?? false
                onTriggered: root.binder.italic = !root.binder.italic
            }
        }
        FormatButton {
            objectName: "underlineButton"
            action: FormatAction {
                text: qsTr("Underline")
                icon.source: "image://icons/fmt_underline"
                shortcut: StandardKey.Underline
                checked: root.binder?.underline ?? false
                onTriggered: root.binder.underline = !root.binder.underline
            }
        }
        FormatButton {
            objectName: "strikeOutButton"
            action: FormatAction {
                text: qsTr("Strikethrough")
                icon.source: "image://icons/fmt_strike"
                shortcut: "Ctrl+D"
                checked: root.binder?.strikeOut ?? false
                onTriggered: root.binder.strikeOut = !root.binder.strikeOut
            }
        }
        FormatButton {
            objectName: "superscriptButton"
            action: FormatAction {
                text: qsTr("Superscript")
                icon.source: "image://icons/fmt_superscript"
                shortcut: "Ctrl+Shift+="
                checked: root.binder?.superscript ?? false
                onTriggered: root.binder.superscript = !root.binder.superscript
            }
        }
        FormatButton {
            objectName: "subscriptButton"
            action: FormatAction {
                text: qsTr("Subscript")
                icon.source: "image://icons/fmt_subscript"
                shortcut: "Ctrl+="
                checked: root.binder?.subscript ?? false
                onTriggered: root.binder.subscript = !root.binder.subscript
            }
        }

        ToolSeparator {}

        FormatButton {
            objectName: "alignLeftButton"
            action: FormatAction {
                text: qsTr("Align Left")
                icon.source: "image://icons/fmt_align_left"
                shortcut: "Ctrl+L"
                checked: root.binder?.alignment === Qt.AlignLeft
                onTriggered: root.binder.alignment = Qt.AlignLeft
            }
        }
        FormatButton {
            objectName: "alignCenterButton"
            action: FormatAction {
                text: qsTr("Align Centre")
                icon.source: "image://icons/fmt_align_center"
                shortcut: "Ctrl+E"
                checked: root.binder?.alignment === Qt.AlignHCenter
                onTriggered: root.binder.alignment = Qt.AlignHCenter
            }
        }
        FormatButton {
            objectName: "alignRightButton"
            action: FormatAction {
                text: qsTr("Align Right")
                icon.source: "image://icons/fmt_align_right"
                shortcut: "Ctrl+R"
                checked: root.binder?.alignment === Qt.AlignRight
                onTriggered: root.binder.alignment = Qt.AlignRight
            }
        }
        FormatButton {
            objectName: "alignJustifyButton"
            action: FormatAction {
                text: qsTr("Justify")
                icon.source: "image://icons/fmt_align_justify"
                shortcut: "Ctrl+J"
                checked: root.binder?.alignment === Qt.AlignJustify
                onTriggered: root.binder.alignment = Qt.AlignJustify
            }
        }

        ToolSeparator {}

        FormatButton {
            objectName: "indentButton"
            action: FormatAction {
                text: qsTr("Increase Indent")
                icon.source: "image://icons/fmt_indent"
                shortcut: "Ctrl+M"
                onTriggered: root.binder.indent()
            }
        }
        FormatButton {
            objectName: "outdentButton"
            action: FormatAction {
                text: qsTr("Decrease Indent")
                icon.source: "image://icons/fmt_outdent"
                shortcut: "Ctrl+Shift+M"
                onTriggered: root.binder.outdent()
            }
        }
    }
}
