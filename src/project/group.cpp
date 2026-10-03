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

#include <QJsonObject>
#include <QPair>
#include <QRegularExpression>
#include <QString>

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

/**! @brief Write the group's entry in the project structure.
 *
 * The documents are not included, as they are stored in the group's file.
 */
void Group::pack(QJsonObject &data, int order) const
{
    data["m:class"_L1] = classToString(m_class);
    data["m:order"_L1] = order;
    data["m:file"_L1] = m_fileName;
    data["u:name"_L1] = m_name;
}

/**! @brief Create a group from its entry in the project structure.
 *
 * The documents are added when the group's file is read. A group with an
 * invalid class is skipped, and nullptr is returned. A file name that is not
 * valid is left empty, so a new one can be given to the group.
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

    const QString fileName = JsonUtils::getJsonString(data, "m:file"_L1, "");

    Group *group = new Group(name, itemClass);
    group->setFileName(isFileName(fileName) ? fileName : QString());
    group->setModified(false);
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

/**! @brief Check if a string is a valid group file name, like
 * "document1.json".
 */
bool Group::isFileName(const QString &value)
{
    static const QRegularExpression pattern(u"^document[1-9][0-9]*\\.json$"_s);
    return pattern.match(value).hasMatch();
}

} // namespace Collett
