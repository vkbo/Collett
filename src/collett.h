/*
** Collett - Collett Main Header
** =============================
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

#pragma once

#define COL_VERSION_STR "0.0.1-alpha1"
#define COL_VERSION_NUM 0x000001a1
#define COL_VERSION_DATE "2025-02-23"

#include <QDebug>
#include <QObject>
#include <QTextFormat>
#include <QtQml/qqmlregistration.h>

namespace Collett {
Q_NAMESPACE
QML_ELEMENT

// Item Enums
// The class of a project group, and the structure level of a document.
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
Q_ENUM_NS(ItemClass)

enum ItemLevel
{
    // The int order is the structure order, from the top
    PartitionLevel = 0,
    ChapterLevel = 1,
    SceneLevel = 2,
};
Q_ENUM_NS(ItemLevel)

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
