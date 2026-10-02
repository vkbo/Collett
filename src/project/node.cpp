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

#include "node.h"
#include "tools.h"
#include "tree.h"

#include <QJsonObject>
#include <QString>

using namespace Qt::Literals::StringLiterals;

namespace Collett {

// Constructor/Destructor
// ======================

Node::Node(const QString &handle, const QString &name, ItemLevel level) : m_handle(handle), m_name(name), m_level(level) {}

Node::~Node() {}

// Public Methods
// ==============

/**! @brief Write the node to a JSON object.
 *
 * Only partitions and chapters can be folded, so only they store whether
 * they are expanded.
 */
void Node::pack(QJsonObject &data, int order) const
{
    data["m:handle"_L1] = m_handle;
    data["m:level"_L1] = levelToString(m_level);
    data["m:order"_L1] = order;
    data["m:characters"_L1] = m_counts.characters;
    data["m:words"_L1] = m_counts.words;
    if (m_level != ItemLevel::SceneLevel) {
        data["m:expanded"_L1] = m_expanded;
    }
    data["u:name"_L1] = m_name;
}

/**! @brief Create a node from a JSON object.
 *
 * A node needs a valid handle and level. If either is missing or invalid,
 * the node is skipped and nullptr is returned.
 */
Node *Node::unpack(const QJsonObject &data)
{
    QString handle = JsonUtils::getJsonString(data, "m:handle"_L1, "");
    if (!Tree::isHandle(handle)) {
        qWarning() << "Skipping project item with invalid handle" << handle;
        return nullptr;
    }

    ItemLevel level = ItemLevel::SceneLevel;
    if (!levelFromString(JsonUtils::getJsonString(data, "m:level"_L1, ""), level)) {
        qWarning() << "Skipping project item with invalid level" << handle;
        return nullptr;
    }

    QString name = JsonUtils::getJsonString(data, "u:name"_L1, "").simplified();
    if (name.isEmpty()) {
        name = tr("Unnamed");
    }

    TextCounts counts;
    counts.words = qMax(data["m:words"_L1].toInt(), 0);
    counts.characters = qMax(data["m:characters"_L1].toInt(), 0);

    Node *node = new Node(handle, name, level);
    node->setCounts(counts);
    node->setExpanded(data["m:expanded"_L1].toBool(true));
    return node;
}

// Static Methods
// ==============

bool Node::levelFromString(const QString &value, ItemLevel &itemLevel)
{
    if (value == "Partition") {
        itemLevel = ItemLevel::PartitionLevel;
        return true;
    }
    if (value == "Chapter") {
        itemLevel = ItemLevel::ChapterLevel;
        return true;
    }
    if (value == "Scene") {
        itemLevel = ItemLevel::SceneLevel;
        return true;
    }
    return false;
}

QString Node::levelToString(ItemLevel itemLevel)
{
    switch (itemLevel) {
    case ItemLevel::PartitionLevel: return "Partition"_L1;
    case ItemLevel::ChapterLevel: return "Chapter"_L1;
    case ItemLevel::SceneLevel: return "Scene"_L1;
    }
    return "Scene"_L1;
}

} // namespace Collett
