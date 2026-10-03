/*
** Collett - Project Node Class
** ============================
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

#include "collett.h"
#include "counting.h"

#include <QJsonObject>
#include <QString>

namespace Collett {

/**! @brief A document entry in a project group.
 *
 * The documents of a group form a flat list in reading order. The structure
 * comes from the level of each document, so a chapter covers the scenes that
 * follow it, up to the next chapter or partition. The title is stored here,
 * not in the document, and may be empty.
 */
class Node
{
public:
    Node(const QString &handle, const QString &title, ItemLevel level);
    ~Node();

    // Methods
    void pack(QJsonObject &data) const;
    static Node *unpack(const QJsonObject &data);

    // Getters
    QString handle() const { return m_handle; };
    QString title() const { return m_title; };
    QString createdTime() const { return m_createdTime; };
    QString updatedTime() const { return m_updatedTime; };
    ItemLevel itemLevel() const { return m_level; };
    TextCounts counts() const { return m_counts; };
    bool isExpanded() const { return m_expanded; };
    bool isFoldable() const { return m_level == ItemLevel::PartitionLevel || m_level == ItemLevel::ChapterLevel; };
    bool hasHardBreak() const { return m_hardBreak && m_level == ItemLevel::SceneLevel; };
    bool isNumbered() const { return m_numbered || m_level != ItemLevel::ChapterLevel; };

    // Setters
    void setTitle(const QString &title) { m_title = title; };
    void setCreatedTime(const QString &time) { m_createdTime = time; };
    void setUpdatedTime(const QString &time) { m_updatedTime = time; };
    void setLevel(ItemLevel level) { m_level = level; };
    void setCounts(const TextCounts &counts) { m_counts = counts; };
    void setExpanded(bool state) { m_expanded = state; };
    void setHardBreak(bool state) { m_hardBreak = state; };
    void setNumbered(bool state) { m_numbered = state; };

    // Static Methods
    static bool levelFromString(const QString &value, ItemLevel &itemLevel);
    static QString levelToString(ItemLevel itemLevel);

private:
    QString m_handle;
    QString m_title;
    QString m_createdTime;
    QString m_updatedTime;
    ItemLevel m_level;
    TextCounts m_counts;
    bool m_expanded = true;
    bool m_hardBreak = false;
    bool m_numbered = true;
};
} // namespace Collett
