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
import QtQuick.Dialogs
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
    readonly property list<SettingsSection> sections: [generalSection, appearanceSection, editingSection, spellingSection]

    // The font picked, until it is saved
    property font textFont

    title: qsTr("Preferences")
    minimumWidth: 600
    minimumHeight: 500
    modality: Qt.ApplicationModal

    /**! Show the dialog with the saved values, at the first section.
     */
    function openDialog() {
        guiLanguage.model = Settings.guiLanguages();
        guiLanguage.currentIndex = guiLanguage.indexOfValue(Settings.guiLanguage);
        textFont = Settings.textFont;
        nativeFontDialog.checked = Settings.nativeFontDialog;
        autoSave.value = Settings.editorAutoSave;
        tabWidth.value = Math.round(Settings.textTabWidth);
        spellLanguage.model = Settings.spellLanguages();
        spellLanguage.currentIndex = spellLanguage.indexOfValue(Settings.spellLanguage);

        width = Settings.prefsWindowSize.width;
        height = Settings.prefsWindowSize.height;
        flow.contentY = 0;
        currentIndex = 0;
        show();
        raise();
        requestActivate();
    }

    /**! Save the values in the form and close the dialog.
     */
    function save() {
        if (guiLanguage.currentIndex >= 0) Settings.guiLanguage = guiLanguage.currentValue;
        Settings.textFont = textFont;
        Settings.nativeFontDialog = nativeFontDialog.checked;
        Settings.editorAutoSave = autoSave.value;
        Settings.textTabWidth = tabWidth.value;
        if (spellLanguage.currentIndex >= 0) Settings.spellLanguage = spellLanguage.currentValue;
        close();
    }

    /**! Scroll a section to the top of the page.
     */
    function showSection(index: int) {
        currentIndex = index;
        scrollAnimation.to = Math.min(sections[index].y, flow.contentHeight - flow.height);
        scrollAnimation.restart();
    }

    // The side bar follows the scrolling, but not while it is scrolling to
    // the section that was clicked
    function updateCurrent() {
        if (scrollAnimation.running) return;
        let found = 0;
        for (let i = 0; i < sections.length; ++i) {
            if (sections[i].y <= flow.contentY + 1) found = i;
        }
        currentIndex = found;
    }

    // The window size is kept however the dialog is closed
    onVisibleChanged: {
        if (visible) return;
        Settings.prefsWindowSize = Qt.size(width, height);
        Settings.flushSettings();
    }

    Shortcut {
        sequences: [StandardKey.Cancel]
        onActivated: root.close()
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

        footer: DialogButtonBox {
            standardButtons: DialogButtonBox.Save | DialogButtonBox.Cancel
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
                        group: qsTr("Application")
                        iconName: "settings"
                    }
                    ListElement {
                        name: qsTr("Appearance")
                        group: qsTr("Application")
                        iconName: "theme_auto"
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

            Flickable {
                id: flow

                objectName: "preferencesFlow"

                Layout.fillWidth: true
                Layout.fillHeight: true
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
                        id: appearanceSection

                        title: qsTr("Appearance")

                        SettingsGroup {
                            title: qsTr("Fonts")

                            SettingsRow {
                                title: qsTr("Text font")
                                help: qsTr("The font used for document text in the editor.")

                                TextField {
                                    objectName: "textFont"

                                    implicitWidth: 220
                                    readOnly: true
                                    text: Settings.fontDescription(root.textFont)
                                    Accessible.name: qsTr("Text font")
                                }
                                Button {
                                    icon.source: "image://icons/font"
                                    icon.color: palette.buttonText
                                    Accessible.name: qsTr("Select Font")
                                    onClicked: fontDialog.open()
                                }
                            }
                            SettingsRow {
                                title: qsTr("Use the system's font selection dialog")
                                help: qsTr("Turn off to use the Qt font dialog, which may have more options.")

                                Switch {
                                    id: nativeFontDialog

                                    Accessible.name: qsTr("Use the system's font selection dialog")
                                }
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

    FontDialog {
        id: fontDialog

        title: qsTr("Select Font")
        currentFont: root.textFont
        options: nativeFontDialog.checked ? 0 : FontDialog.DontUseNativeDialog
        onAccepted: root.textFont = selectedFont
    }
}
