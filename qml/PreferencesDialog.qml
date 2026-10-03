/*
** Collett - Preferences Dialog
** ============================
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

// All settings in one scrolling page of sections. The side bar jumps to a
// section, and follows the section at the top of the page. Changes are only
// kept when saved.
Window {
    id: root

    objectName: "preferencesDialog"

    // The index of the section at the top of the page
    property int currentIndex: 0
    readonly property list<SettingsSection> sections: [generalSection, fontsSection, editingSection, spellingSection]

    // The fonts picked, until they are saved
    property font guiFont
    property font textFont
    property font headingFont
    property font monoFont

    title: qsTr("Preferences")
    minimumWidth: 600
    minimumHeight: 500
    modality: Qt.ApplicationModal
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
    flags: Qt.Dialog
    color: palette.window

    /**! Show the dialog with the saved values, at the first section.
     */
    function openDialog() {
        guiLanguage.model = Settings.guiLanguages();
        guiLanguage.currentIndex = guiLanguage.indexOfValue(Settings.guiLanguage);
        guiFont = Settings.guiFont;
        textFont = Settings.textFont;
        headingFont = Settings.headingFont;
        monoFont = Settings.monoFont;
        autoSave.value = Settings.editorAutoSave;
        tabWidth.value = Math.round(Settings.textTabWidth);
        autoIndent.checked = Settings.textAutoIndent;
        spellLanguage.model = Settings.spellLanguages();
        spellLanguage.currentIndex = spellLanguage.indexOfValue(Settings.spellLanguage);

        width = Settings.prefsWindowSize.width;
        height = Settings.prefsWindowSize.height;

        // Centre on the main window, as not all window managers place it
        const owner = transientParent;
        if (owner) {
            x = owner.x + Math.round((owner.width - width) / 2);
            y = owner.y + Math.round((owner.height - height) / 2);
        }
        stack.popToIndex(0, StackView.Immediate);
        flow.contentY = 0;
        currentIndex = 0;
        show();
        raise();
        requestActivate();
    }

    /**! Save the values in the form and close the dialog.
     */
    function save() {
        if (guiLanguage.currentIndex >= 0)
            Settings.guiLanguage = guiLanguage.currentValue;
        Settings.guiFont = guiFont;
        Settings.textFont = textFont;
        Settings.headingFont = headingFont;
        Settings.monoFont = monoFont;
        Settings.editorAutoSave = autoSave.value;
        Settings.textTabWidth = tabWidth.value;
        Settings.textAutoIndent = autoIndent.checked;
        if (spellLanguage.currentIndex >= 0)
            Settings.spellLanguage = spellLanguage.currentValue;
        close();
    }

    /**! Open the font page for one of the fonts.
     */
    function editFont(key: string, title: string) {
        stack.push(fontPage, {
            key: key,
            title: title,
            initialFont: root[key],
            fixedPitch: key === "monoFont"
        });
    }

    /**! Scroll a section to the top of the page.
     */
    function showSection(index: int) {
        if (stack.depth > 1)
            stack.popToIndex(0);
        currentIndex = index;
        scrollAnimation.to = Math.min(sections[index].y, flow.contentHeight - flow.height);
        scrollAnimation.restart();
    }

    // The side bar follows the scrolling, but not while it is scrolling to
    // the section that was clicked
    function updateCurrent() {
        if (scrollAnimation.running)
            return;
        let found = 0;
        for (let i = 0; i < sections.length; ++i) {
            if (sections[i].y <= flow.contentY + 1)
                found = i;
        }
        currentIndex = found;
    }

    // The window size is kept however the dialog is closed
    onVisibleChanged: {
        if (visible)
            return;
        Settings.prefsWindowSize = Qt.size(width, height);
        Settings.flushSettings();
    }

    Shortcut {
        sequences: [StandardKey.Cancel]
        onActivated: stack.depth > 1 ? stack.pop() : root.close()
    }

    Page {
        anchors.fill: parent
        padding: 12
        title: root.title

        header: Label {
            padding: 12
            bottomPadding: 0
            text: root.title
            font.pointSize: Application.font.pointSize * 1.4
        }

        // Material rounds the corners to fit in a popup, which a window does
        // not have
        footer: DialogButtonBox {
            standardButtons: DialogButtonBox.Save | DialogButtonBox.Cancel
            Material.roundedScale: Material.NotRounded
            onAccepted: root.save()
            onRejected: root.close()

            Component.onCompleted: {
                const saveButton = standardButton(DialogButtonBox.Save);
                saveButton.objectName = "saveButton";
                saveButton.icon.source = "image://icons/btn_save";
                const cancelButton = standardButton(DialogButtonBox.Cancel);
                cancelButton.icon.source = "image://icons/btn_cancel";
            }
        }

        RowLayout {
            anchors.fill: parent
            spacing: 16

            // The side bar lists the sections under group headings
            ListView {
                id: sideBar

                Layout.preferredWidth: 220
                Layout.fillHeight: true
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                currentIndex: root.currentIndex

                model: ListModel {
                    ListElement {
                        name: qsTr("General")
                        group: qsTr("Appearance")
                        iconName: "settings"
                    }
                    ListElement {
                        name: qsTr("Fonts")
                        group: qsTr("Appearance")
                        iconName: "font"
                    }
                    ListElement {
                        name: qsTr("Editing")
                        group: qsTr("Editor")
                        iconName: "prj_document"
                    }
                    ListElement {
                        name: qsTr("Spell Checking")
                        group: qsTr("Editor")
                        iconName: "language"
                    }
                }

                section.property: "group"
                section.delegate: Label {
                    required property string section

                    width: ListView.view.width
                    leftPadding: 16
                    topPadding: 12
                    bottomPadding: 4
                    text: section
                    font.pointSize: Application.font.pointSize * 0.9
                    opacity: 0.7
                }

                delegate: ItemDelegate {
                    required property int index
                    required property string name
                    required property string iconName

                    objectName: "sideBarButton"
                    width: ListView.view.width
                    text: name
                    icon.source: "image://icons/" + iconName
                    highlighted: ListView.isCurrentItem
                    onClicked: root.showSection(index)
                }
            }

            // Pages for single settings are pushed on top of the settings
            StackView {
                id: stack

                objectName: "preferencesStack"

                Layout.fillWidth: true
                Layout.fillHeight: true
                initialItem: flow

                Flickable {
                    id: flow

                    objectName: "preferencesFlow"

                    clip: true
                    contentWidth: width
                    contentHeight: sectionColumn.height
                    boundsBehavior: Flickable.StopAtBounds
                    onContentYChanged: root.updateCurrent()

                    ScrollBar.vertical: ScrollBar {}

                    NumberAnimation {
                        id: scrollAnimation

                        target: flow
                        property: "contentY"
                        duration: 250
                        easing.type: Easing.OutCubic
                    }

                    Column {
                        id: sectionColumn

                        width: flow.width - 16

                        SettingsSection {
                            id: generalSection

                            title: qsTr("General")

                            SettingsGroup {
                                title: qsTr("Language")

                                SettingsRow {
                                    title: qsTr("Interface language")
                                    help: qsTr("The language of menus, labels and messages. Requires a restart.")

                                    ComboBox {
                                        id: guiLanguage

                                        objectName: "guiLanguage"

                                        implicitWidth: 220
                                        textRole: "text"
                                        valueRole: "value"
                                        Accessible.name: qsTr("Interface language")
                                    }
                                }
                            }
                        }

                        SettingsSection {
                            id: fontsSection

                            title: qsTr("Fonts")

                            SettingsGroup {
                                title: qsTr("Interface")

                                SettingsLinkRow {
                                    objectName: "guiFontRow"

                                    title: qsTr("Interface font")
                                    help: qsTr("The font used for menus, labels and buttons.")
                                    value: Fonts.describe(root.guiFont)
                                    onClicked: root.editFont("guiFont", title)
                                }
                            }

                            SettingsGroup {
                                title: qsTr("Editor")

                                SettingsLinkRow {
                                    objectName: "textFontRow"

                                    title: qsTr("Text font")
                                    help: qsTr("The font used for document text in the editor.")
                                    value: Fonts.describe(root.textFont)
                                    onClicked: root.editFont("textFont", title)
                                }
                                SettingsLinkRow {
                                    objectName: "headingFontRow"

                                    title: qsTr("Heading font")
                                    help: qsTr("The font used for titles and headings in the editor. Larger headings are scaled from its size.")
                                    value: Fonts.describe(root.headingFont)
                                    onClicked: root.editFont("headingFont", title)
                                }
                                SettingsLinkRow {
                                    objectName: "monoFontRow"

                                    title: qsTr("Monospace font")
                                    help: qsTr("The fixed width font used in the editor.")
                                    value: Fonts.describe(root.monoFont)
                                    onClicked: root.editFont("monoFont", title)
                                }
                            }
                        }

                        SettingsSection {
                            id: editingSection

                            title: qsTr("Editing")

                            SettingsGroup {
                                title: qsTr("Saving")

                                SettingsRow {
                                    title: qsTr("Auto-save interval")
                                    help: qsTr("How often, in seconds, changed documents are saved.")

                                    SpinBox {
                                        id: autoSave

                                        objectName: "autoSave"

                                        from: 5
                                        to: 600
                                        editable: true
                                        Accessible.name: qsTr("Auto-save interval")
                                    }
                                }
                            }

                            SettingsGroup {
                                title: qsTr("Text")

                                SettingsRow {
                                    title: qsTr("First line indent")
                                    help: qsTr("The width of the indent at the start of indented paragraphs.")

                                    SpinBox {
                                        id: tabWidth

                                        from: 0
                                        to: 200
                                        editable: true
                                        Accessible.name: qsTr("First line indent")
                                    }
                                }

                                SettingsRow {
                                    title: qsTr("Indent paragraphs automatically")
                                    help: qsTr("New paragraphs get a first line indent, except after a heading, or when centred or right-aligned.")

                                    Switch {
                                        id: autoIndent

                                        objectName: "autoIndent"

                                        Accessible.name: qsTr("Indent paragraphs automatically")
                                    }
                                }
                            }
                        }

                        SettingsSection {
                            id: spellingSection

                            title: qsTr("Spell Checking")

                            SettingsGroup {
                                title: qsTr("Language")

                                SettingsRow {
                                    title: qsTr("Spell checking language")
                                    help: qsTr("The dictionary used for spell checking, unless a project has its own.")

                                    ComboBox {
                                        id: spellLanguage

                                        objectName: "spellLanguage"

                                        implicitWidth: 220
                                        textRole: "text"
                                        valueRole: "value"
                                        Accessible.name: qsTr("Spell checking language")
                                    }
                                }
                            }
                        }

                        // Room for the last section to scroll to the top
                        Item {
                            width: 1
                            height: Math.max(0, flow.height - spellingSection.height)
                        }
                    }
                }
            }
        }
    }

    Component {
        id: fontPage

        FontPage {
            // The dialog property the page edits
            property string key

            onChosen: selected => root[key] = selected
            onBack: stack.pop()
        }
    }
}
