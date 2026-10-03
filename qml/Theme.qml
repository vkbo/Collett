/*
** Collett - Theme
** ===============
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

pragma Singleton

import QtQuick
import QtQuick.Controls.Material

import Collett

QtObject {
    // Set by the main window from the theme Material resolved
    property bool dark: false

    // The Material theme for the theme mode setting
    readonly property int materialTheme: {
        switch (Settings.themeMode) {
        case Settings.LightTheme:
            return Material.Light;
        case Settings.DarkTheme:
            return Material.Dark;
        default:
            return Material.System;
        }
    }

    /**! Switch to the next theme mode: follow the system, light, then dark.
     */
    function nextThemeMode() {
        Settings.themeMode = (Settings.themeMode + 1) % 3;
        Settings.flushSettings();
    }

    // Colours from the Material palette: a darker shade on light backgrounds,
    // and a lighter one on dark backgrounds
    readonly property int strongShade: dark ? Material.Shade300 : Material.Shade700
    readonly property int greyShade: dark ? Material.Shade400 : Material.Shade600

    readonly property color partitionColor: Material.color(Material.Green, strongShade)
    readonly property color chapterColor: Material.color(Material.Red, strongShade)
    readonly property color sceneColor: Material.color(Material.Blue, strongShade)
    readonly property color pageColor: Material.color(Material.Grey, greyShade)

    // The line under misspelled words
    readonly property color spellErrorColor: Material.color(Material.Red, dark ? Material.Shade300 : Material.Shade600)
    readonly property color formatErrorColor: Material.color(Material.Orange, dark ? Material.Shade300 : Material.Shade600)

    // The icon of a level, from the icon theme
    function levelIcon(level: int): string {
        switch (level) {
        case Collett.PartitionLevel:
            return "prj_title";
        case Collett.ChapterLevel:
            return "prj_chapter";
        case Collett.PageLevel:
            return "prj_document";
        default:
            return "prj_scene";
        }
    }

    function levelColor(level: int): color {
        switch (level) {
        case Collett.PartitionLevel:
            return partitionColor;
        case Collett.ChapterLevel:
            return chapterColor;
        case Collett.PageLevel:
            return pageColor;
        default:
            return sceneColor;
        }
    }
}
