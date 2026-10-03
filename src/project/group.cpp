/*
** Collett - Project Group Class
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

#include "group.h"
#include "tools.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QPair>
#include <QString>

#include <algorithm>

using namespace Qt::Literals::StringLiterals;

namespace Collett {

// Constructor/Destructor
// ======================

Group::Group(const QString &name, ItemClass itemClass) : m_name(name), m_class(itemClass) {}

Group::~Group()
{
    qDeleteAll(m_items);
}

// Public Methods
// ==============

void Group::pack(QJsonObject &data, int order) const
{
    QJsonArray items;
    for (qsizetype i = 0; i < m_items.size(); ++i) {
        QJsonObject item;
        m_items.at(i)->pack(item, i);
        items.append(item);
    }

    data["m:class"_L1] = classToString(m_class);
    data["m:order"_L1] = order;
    data["u:name"_L1] = m_name;
    data["x:items"_L1] = items;
}

/**! @brief Create a group and its documents from a JSON object.
 *
 * The documents are sorted by their order value. Documents without one keep
 * their position in the array. Invalid documents are skipped. A group with
 * an invalid class is skipped entirely, and nullptr is returned.
 */
Group *Group::unpack(const QJsonObject &data)
{
    ItemClass itemClass = ItemClass::NovelClass;
    if (!classFromString(JsonUtils::getJsonString(data, "m:class"_L1, ""), itemClass)) {
        qWarning() << "Skipping project group with invalid class";
        return nullptr;
    }

    QString name = JsonUtils::getJsonString(data, "u:name"_L1, "").simplified();
    if (name.isEmpty()) {
        name = tr("Unnamed");
    }

    QList<QPair<int, Node *>> ordered;
    const QJsonArray items = data["x:items"_L1].toArray();
    for (qsizetype i = 0; i < items.size(); ++i) {
        if (Node *node = Node::unpack(items.at(i).toObject())) {
            ordered.append({items.at(i)["m:order"_L1].toInt(int(i)), node});
        }
    }
    std::stable_sort(ordered.begin(), ordered.end(), [](const auto &a, const auto &b) { return a.first < b.first; });

    Group *group = new Group(name, itemClass);
    for (const auto &entry : ordered) {
        group->appendItem(entry.second);
    }
    return group;
}

// Static Methods
// ==============

bool Group::classFromString(const QString &value, ItemClass &itemClass)
{
    static const QList<QPair<QString, ItemClass>> classes = {
        {"Novel", ItemClass::NovelClass},
        {"Character", ItemClass::CharacterClass},
        {"Plot", ItemClass::PlotClass},
        {"Location", ItemClass::LocationClass},
        {"Object", ItemClass::ObjectClass},
        {"Entity", ItemClass::EntityClass},
        {"Custom", ItemClass::CustomClass},
        {"Archive", ItemClass::ArchiveClass},
        {"Trash", ItemClass::TrashClass},
    };
    for (const auto &entry : classes) {
        if (entry.first == value) {
            itemClass = entry.second;
            return true;
        }
    }
    return false;
}

QString Group::classToString(ItemClass itemClass)
{
    switch (itemClass) {
    case ItemClass::NovelClass: return "Novel"_L1;
    case ItemClass::CharacterClass: return "Character"_L1;
    case ItemClass::PlotClass: return "Plot"_L1;
    case ItemClass::LocationClass: return "Location"_L1;
    case ItemClass::ObjectClass: return "Object"_L1;
    case ItemClass::EntityClass: return "Entity"_L1;
    case ItemClass::CustomClass: return "Custom"_L1;
    case ItemClass::ArchiveClass: return "Archive"_L1;
    case ItemClass::TrashClass: return "Trash"_L1;
    }
    return "Novel"_L1;
}

} // namespace Collett
