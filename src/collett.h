/*
** Collett - Collett Main Header
** =============================
**
** This file is a part of Collett
** Copyright (C) 2025 Veronica Berglyd Olsen
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

#pragma once

#define COL_VERSION_STR "0.0.1-alpha1"
#define COL_VERSION_NUM 0x000001a1
#define COL_VERSION_DATE "2025-02-23"

#define PROJECT_ITEM_MIME "text/vnd.collett.handle"

#include <QDebug>
#include <QObject>
#include <QTextFormat>

namespace Collett {

// Item Enums
// Used for distinguishing between the roles of a project node.
enum ItemType
{
    InvisibleRoot,
    RootType,
    FolderType,
    FileType,
};
enum ItemClass
{
    NovelClass = 0,
    CharacterClass = 1,
    PlotClass = 2,
    LocationClass = 3,
    ObjectClass = 4,
    EntityClass = 5,
    CustomClass = 6,
    ArchiveClass = 7,
    TrashClass = 8,
};
enum ItemLevel
{
    // The int order here matters and must not be changed
    // See: ProjectModel::addFile()
    PageLevel = 0,
    TitleLevel = 1,
    ChapterLevel = 2,
    SceneLevel = 3,
    NoteLevel = 4,
};

// Theme Colours
// Used as index keys to look up colours from the Theme class.
enum ThemeColor
{
    RootColor = 0,
    FolderColor = 1,
    FileColor = 2,
    TitleColor = 3,
    ChapterColor = 4,
    SceneColor = 5,
    NoteColor = 6,
    DefaultColor = 7,
    FadedColor = 8,
    Red = 9,
    Orange = 10,
    Yellow = 11,
    Green = 12,
    Aqua = 13,
    Blue = 14,
    Purple = 15,

    // Icons
    ToolColor = 16,
    AcceptColor = 17,
    RejectColor = 18,
    ActionColor = 19,
    OptionColor = 20,
    ApplyColor = 21,
    CreateColor = 22,
    DestroyColor = 23,
    ResetColor = 24,
    AddColor = 25,
    ChangeColor = 26,
    RemoveColor = 27,
};

// Syntax Colours
// Used as index keys to look up editor highlighting colours from the Theme class.
enum SyntaxColor
{
    SyntaxHeader = 0,
    SyntaxEmphasis = 1,
    SyntaxComment = 2,
    SyntaxSpellLine = 3,
    SyntaxErrorLine = 4,
};

// Theme Mode
// Which of the light and dark themes is loaded.
#define COL_DEFAULT_LIGHT_THEME "default_light"
#define COL_DEFAULT_DARK_THEME "default_dark"
enum ThemeMode
{
    AutoTheme = 0,
    LightTheme = 1,
    DarkTheme = 2,
};

// Tool Buttons
// Icon-only buttons with a shared tool tip and icon, created by the Theme class.
enum ToolButton
{
    FontButton = 0,
};

// Standard Buttons
// Dialog buttons with a shared label and icon, created by the Theme class.
enum StandardButton
{
    SaveButton = 0,
    CancelButton = 1,
    OkButton = 2,
};

// Text Format Properties
// Custom properties stored on the block formats of a document.
enum TextProperty
{
    BlockTypeProperty = QTextFormat::UserProperty + 1,
};
enum BlockType
{
    TextBlock = 0,
    CommentBlock = 1,
};

enum JsonUtilsError
{
    NoError,
    FileError,
    JsonError
};

} // namespace Collett
