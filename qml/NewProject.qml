/*
** Collett - New Project
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

import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import Collett

// The form for creating a new project, shown when no project is open. The
// project is made in a folder named after it, inside the chosen location.
Item {
    id: root

    property Project project

    signal openRequested

    // A file URL as a local path, for showing in the location field
    function localPath(url: url): string {
        return decodeURIComponent(url.toString().replace(/^file:\/\//, ""));
    }

    function focusName() {
        name.forceActiveFocus();
    }

    function create() {
        error.text = project.createProject(location.text, name.text);
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(root.width - 64, 480)
        spacing: 12

        Label {
            text: qsTr("New Project")
            font.pointSize: Application.font.pointSize * 1.6
            font.bold: true
        }

        Label {
            text: qsTr("Name")
        }
        TextField {
            id: name

            objectName: "nameField"

            Layout.fillWidth: true
            placeholderText: qsTr("The title of the novel")
            focus: true
            onAccepted: root.create()
        }

        Label {
            text: qsTr("Location")
        }
        RowLayout {
            Layout.fillWidth: true

            TextField {
                id: location

                objectName: "locationField"

                Layout.fillWidth: true
                text: root.localPath(StandardPaths.writableLocation(StandardPaths.DocumentsLocation))
                onAccepted: root.create()
            }
            Button {
                text: qsTr("Browse")
                onClicked: folderDialog.open()
            }
        }

        Label {
            Layout.fillWidth: true
            text: name.text.trim() ? qsTr("The project will be created in a folder named \"%1\" in this location.").arg(name.text.trim()) : ""
            wrapMode: Text.Wrap
            opacity: 0.7
        }

        Label {
            id: error

            Layout.fillWidth: true
            wrapMode: Text.Wrap
            color: Theme.chapterColor
            visible: text !== ""
        }

        RowLayout {
            Layout.fillWidth: true

            Button {
                objectName: "openButton"
                text: qsTr("Open Project…")
                flat: true
                onClicked: root.openRequested()
            }
            Item {
                Layout.fillWidth: true
            }
            Button {
                objectName: "createButton"
                text: qsTr("Create Project")
                enabled: name.text.trim() !== "" && location.text.trim() !== ""
                onClicked: root.create()
            }
        }
    }

    FolderDialog {
        id: folderDialog

        title: qsTr("Choose a Location")
        currentFolder: "file://" + location.text
        onAccepted: location.text = root.localPath(selectedFolder)
    }
}
