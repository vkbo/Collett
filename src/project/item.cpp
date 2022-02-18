/*
** Collett – Project Item Class
** ============================
**
** This file is a part of Collett
** Copyright 2020–2022, Veronica Berglyd Olsen
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

#include "collett.h"
#include "item.h"

#include <QUuid>
#include <QVector>
#include <QVariant>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QVariantList>
#include <QLatin1String>

namespace Collett {

/**!
 * @brief Construct a new Item object.
 *
 * @param label  the label of the new item.
 * @param type   the type of the new item.
 * @param parent the parent of the new item, optional.
 */
Item::Item(const QUuid &uuid, const QString &name, bool story, ItemType type, Item *parent)
    : m_parentItem(parent)
{
    m_childItems = QVector<Item*>{};
    m_story    = story;
    m_handle   = uuid;
    m_name     = name;
    m_type     = type;
    m_words    = 0;
    m_expanded = false;
}

Item::~Item() {
    qDeleteAll(m_childItems);
}

/**
 * Class Methods
 * =============
 */

/**!
 * @brief Add a child item to the current item from user input.
 * 
 * Used for adding a new child item from user input. The item's UUID is generated
 * automatically. All other values are set to their defaults.
 *
 * @param name a label to describe the item.
 * @param type the type of the item.
 * @param pos  the insert position in the child vector. If out of range, the
 *             item is appended.
 * @return a pointer to the newly added item.
 */
Item *Item::addChild(const QString &name, ItemType type, int pos) {
    if (!this->allowedChild(type)) {
        return nullptr;
    }

    Item *item = new Item(QUuid::createUuid(), name, m_story, type, this);
    if (pos >= 0 && pos < m_childItems.size()) {
        m_childItems.insert((qsizetype)pos, item);
    } else {
        m_childItems.append(item);
    }
    return item;
}

/**!
 * @brief Add a child item to the current item from JSON input.
 * 
 * Used for adding a new child item when loading data from a JSON file. Values
 * used for the constructor are validated. Other values will be reset to defaults
 * if they cannot be processed.
 * 
 * @param json the JSON object read from file.
 * @return a pointer to the newly added item.
 */
Item *Item::addChild(const QJsonObject &json) {

    if (json.isEmpty()) {
        return nullptr;
    }

    QUuid    handle   = QUuid();
    QString  name     = "";
    ItemType type     = Item::Invalid;
    int      words    = 0;
    bool     expanded = false;

    if (json.contains(QLatin1String("m:handle"))) {
        handle = QUuid(json[QLatin1String("m:handle")].toString());
    }
    if (json.contains(QLatin1String("u:name"))) {
        name = json[QLatin1String("u:name")].toString();
    }
    if (json.contains(QLatin1String("u:type"))) {
        type = Item::typeFromString(json[QLatin1String("u:type")].toString());
    }
    if (json.contains(QLatin1String("m:words"))) {
        words = json[QLatin1String("m:words")].toInt();
    }
    if (json.contains(QLatin1String("m:expanded"))) {
        expanded = json[QLatin1String("m:expanded")].toBool();
    }

    if (type == Item::Invalid) {
        qWarning() << "Invalid story item type encountered";
        return nullptr;
    }
    if (type == Item::Root) {
        qWarning() << "Only one Root story item is allowed";
        return nullptr;
    }

    if (handle.isNull()) {
        qWarning() << "Cannot add story item with invalid handle";
        return nullptr;
    }

    if (name.isEmpty()) {
        name = tr("Unnamed");
    }

    qDebug() << "Adding item with handle" << handle.toString(QUuid::WithoutBraces);
    if (!this->allowedChild(type)) {
        qWarning() << "This item cannot be added as a child of this parent";
        return nullptr;
    }
    
    Item *item = new Item(handle, name, m_story, type, this);
    item->setWordCount(words);
    item->setExpanded(expanded);
    m_childItems.append(item);

    if (json.contains(QLatin1String("x:items"))) {
        if (json[QLatin1String("x:items")].isArray()) {
            for (const QJsonValue &value : json[QLatin1String("x:items")].toArray()) {
                if (value.isObject()) {
                    item->addChild(value.toObject());
                } else {
                    qWarning() << "Item: Child item is not a JSON object";
                }
            }
        }
    }

    return item;
}

/**!
 * @brief Class method.
 *
 * Pack all member values into a JSON object. This function is recursive, and
 * should only be called on the root item.
 *
 * @return a JSON object.
 */
QJsonObject Item::toJsonObject() {

    QJsonObject item;
    QJsonArray children;

    for (qsizetype i=0; i<m_childItems.size(); ++i) {
        children.append(m_childItems.at(i)->toJsonObject());
    }

    QLatin1String type;
    bool expandable = false;
    switch (m_type) {
        case Item::Root:
            type = QLatin1String("ROOT");
            expandable = false;
            break;
        case Item::Book:
            type = QLatin1String("BOOK");
            expandable = true;
            break;
        case Item::Partition:
            type = QLatin1String("PARTITION");
            expandable = true;
            break;
        case Item::Chapter:
            type = QLatin1String("CHAPTER");
            expandable = true;
            break;
        case Item::Scene:
            type = QLatin1String("SCENE");
            expandable = false;
            break;
        case Item::Page:
            type = QLatin1String("PAGE");
            expandable = false;
            break;
        case Item::Group:
            type = QLatin1String("GROUP");
            expandable = true;
            break;
        case Item::Note:
            type = QLatin1String("NOTE");
            expandable = false;
            break;
        default:
            return QJsonObject();
            expandable = false;
            break;
    }

    if (!m_parentItem) {
        item[QLatin1String("u:type")]  = type;
        item[QLatin1String("x:items")] = children;
    } else {
        item[QLatin1String("m:handle")] = m_handle.toString(QUuid::WithoutBraces);
        item[QLatin1String("m:order")]  = row();
        item[QLatin1String("m:words")]  = m_words;
        item[QLatin1String("u:name")]   = m_name;
        item[QLatin1String("u:type")]   = type;
        if (children.size() > 0) {
            item[QLatin1String("x:items")] = children;
        }
        if (expandable) {
            item[QLatin1String("m:expanded")] = m_expanded;
        }
    }

    return item;
}

/**!
 * @brief Check for allowed child item types of current item.
 *
 * This function determines which child item types an item of a given type is
 * allowed to hold in its list of child elements. This is the authoritative
 * function determining these rules.
 *
 * @param  type the item type to check against.
 * @return true if the item type is allowed, false if not.
 */
bool Item::allowedChild(Item::ItemType type) const {
    if (m_story) {
        switch (m_type) {
            case Item::Root:
                return type == Item::Book;
                break;
            case Item::Book:
                return type == Item::Partition || type == Item::Chapter || type == Item::Page;
                break;
            case Item::Partition:
                return type == Item::Chapter || type == Item::Page;
                break;
            case Item::Chapter:
                return type == Item::Scene;
                break;
            default:
                return false;
        }
    } else {
        switch (m_type) {
            case Item::Root:
                return type == Item::Group || type == Item::Note;
                break;
            case Item::Group:
                return type == Item::Note;
                break;
            default:
                return false;
        }
    }
}

/**!
 * @brief Check for allowed sibling item types of current item.
 *
 * This function uses the @sa allowedChild function against the parent item to
 * validate the request.
 *
 * @param  type the item type to check against.
 * @return true if the item type is allowed, false if not.
 */
bool Item::allowedSibling(Item::ItemType type) const {
    if (m_parentItem) {
        return m_parentItem->allowedChild(type);
    } else {
        return false;
    }
}

bool Item::canHoldDocument() const {
    switch (m_type) {
        case ItemType::Chapter: return true;
        case ItemType::Scene:   return true;
        case ItemType::Page:    return true;
        case ItemType::Note:    return true;
        default: return false;
    }
}

/**
 * Class Setters
 * =============
 */

void Item::setName(const QString &name) {
    m_name = name;
}


void Item::setWordCount(int count) {
    m_words = count > 0 ? count : 0;
}

void Item::setExpanded(bool state) {
    m_expanded = state;
}

/**
 * Class Getters
 * =============
 */

Item::ItemType Item::type() const {
    return m_type;
}

QUuid Item::handle() const {
    return m_handle;
}

QString Item::name() const {
    return m_name;
}

int Item::wordCount() const {
    return m_words;
}

int Item::childWordCounts() const {
    int tCount = 0;
    for (Item* child : m_childItems) {
        tCount += child->childWordCounts();
    }
    return tCount;
}

bool Item::isExpanded() const {
    return m_expanded;
}

Item *Item::findItemFromHandle(const QUuid &uuid) const {
    if (m_handle == uuid) {
        return const_cast<Item*>(this);
    } else {
        for (Item* child : m_childItems) {
            Item *item = child->findItemFromHandle(uuid);
            if (item) return item;
        }
    }
    return nullptr;
}

/**
 * Static Methods
 * ==============
 */

/**!
 * @brief Static method to convert type enum to string label.
 *
 * A static method that will translate the ItemType enum into a localised string
 * that can be displayed on the GUI.
 *
 * @param  type the type to translate.
 * @return a string with the localised name of the type.
 */
QString Item::typeToLabel(ItemType type) {
    QString name = "";
    switch (type) {
        case Item::Root:      name = ""; break;
        case Item::Book:      name = tr("Book"); break;
        case Item::Partition: name = tr("Partition"); break;
        case Item::Chapter:   name = tr("Chapter"); break;
        case Item::Scene:     name = tr("Scene"); break;
        case Item::Page:      name = tr("Page"); break;
        case Item::Group:     name = tr("Group"); break;
        case Item::Note:      name = tr("Note"); break;
        case Item::Invalid:   name = ""; break;
    }
    return name;
}

Item::ItemType Item::typeFromString(const QString &value) {
    QString upper = value.toUpper();
    if (upper == "ROOT") {
        return Item::Root;
    } else if (upper == "BOOK") {
        return Item::Book;
    } else if (upper == "PARTITION") {
        return Item::Partition;
    } else if (upper == "CHAPTER") {
        return Item::Chapter;
    } else if (upper == "SCENE") {
        return Item::Scene;
    } else if (upper == "PAGE") {
        return Item::Page;
    } else if (upper == "GROUP") {
        return Item::Group;
    } else if (upper == "NOTE") {
        return Item::Note;
    } else {
        return Item::Invalid;
    }
}

/**
 * Model Access
 * ============
 */

Item *Item::child(int row) {
    if (row < 0 || row >= m_childItems.size()) {
        return nullptr;
    } else {
        return m_childItems.at(row);
    }
}

int Item::childCount() const {
    return m_childItems.count();
}

int Item::row() const {
    if (m_parentItem) {
        return m_parentItem->m_childItems.indexOf(const_cast<Item*>(this));
    } else {
        return 0;
    }
}

QVariant Item::data() const {
    QVariantList itemData;
    itemData << m_name << m_words << typeToLabel(m_type);
    return QVariant::fromValue(itemData);
}

Item *Item::parentItem() {
    return m_parentItem;
}

} // namespace Collett
