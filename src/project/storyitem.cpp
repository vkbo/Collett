/*
** Collett – Project Story Item Class
** ==================================
**
** This file is a part of Collett
** Copyright 2020–2021, Veronica Berglyd Olsen
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
#include "storyitem.h"

#include <QUuid>
#include <QVector>
#include <QVariant>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QVariantList>

namespace Collett {

/**!
 * @brief Construct a new StoryItem object.
 *
 * @param label the label of the new item.
 * @param type  the type of the new item.
 * @param parent the parent of the new item, optional.
 */
StoryItem::StoryItem(const QString &name, ItemType type, StoryItem *parent)
    : m_parentItem(parent)
{
    m_handle = QUuid::createUuid();
    m_name   = name;
    m_type   = type;
    m_wCount = 1234;
}

StoryItem::~StoryItem() {
    qDeleteAll(m_childItems);
}

/**
 * Class Methods
 * =============
 */

/**!
 * @brief Class Method
 *
 * Add a child item to the current item.
 *
 * @param name a label to describe the item.
 * @param type the type of the item.
 * @param pos  the insert position in the child vector. If out of range, the
 *             item is appended.
 * @return     A pointer to the newly added item.
 */
StoryItem *StoryItem::addChild(const QString &name, ItemType type, int pos) {
    if (!this->allowedChild(type)) {
        return nullptr;
    }

    StoryItem *item = new StoryItem(name, type, this);
    if (pos >= 0 && pos < m_childItems.size()) {
        m_childItems.insert((qsizetype)pos, item);
    } else {
        m_childItems.append(item);
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
QJsonObject StoryItem::toJsonObject() {

    QJsonObject item;
    QJsonArray children;

    for (qsizetype i=0; i<m_childItems.size(); ++i) {
        children.append(m_childItems.at(i)->toJsonObject());
    }

    QString type = "UNKNOWN";
    switch (m_type) {
        case StoryItem::Root:      type = "ROOT"; break;
        case StoryItem::Book:      type = "BOOK"; break;
        case StoryItem::Partition: type = "PARTITION"; break;
        case StoryItem::Chapter:   type = "CHAPTER"; break;
        case StoryItem::Scene:     type = "SCENE"; break;
        case StoryItem::Page:      type = "PAGE"; break;
    }

    if (!m_parentItem) {
        item["type"]   = type;
        item["xItems"] = children;
    } else {
        item["handle"] = m_handle.toString(QUuid::WithoutBraces);
        item["name"]   = m_name;
        item["type"]   = type;
        item["order"]  = row();
        item["wCount"] = m_wCount;
        if (children.size() > 0) {
            item["xItems"] = children;
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
bool StoryItem::allowedChild(StoryItem::ItemType type) const {
    switch (m_type) {
        case StoryItem::Root:
            return type == StoryItem::Book;
            break;
        case StoryItem::Book:
            return type == StoryItem::Partition || type == StoryItem::Chapter || type == StoryItem::Page;
            break;
        case StoryItem::Partition:
            return type == StoryItem::Chapter || type == StoryItem::Page;
            break;
        case StoryItem::Chapter:
            return type == StoryItem::Scene;
            break;
        default:
            return false;
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
bool StoryItem::allowedSibling(StoryItem::ItemType type) const {
    if (m_parentItem) {
        return m_parentItem->allowedChild(type);
    } else {
        return false;
    }
}

/**
 * Class Setters
 * =============
 */

void StoryItem::setName(const QString &name) {
    m_name = name;
}


void StoryItem::setWordCount(int count) {
    m_wCount = count > 0 ? count : 0;
}

/**
 * Class Getters
 * =============
 */

StoryItem::ItemType StoryItem::type() const {
    return m_type;
}

QString StoryItem::name() const {
    return m_name;
}

int StoryItem::wordCount() const {
    return m_wCount;
}

int StoryItem::childWordCounts() const {
    int tCount = 0;
    for (StoryItem* child : m_childItems) {
        tCount += child->childWordCounts();
    }
    return tCount;
}

/**
 * Static Methods
 * ==============
 */

/**!
 * @brief Static method to convert type enum to string.
 *
 * A static method that will translate the ItemType enum into a localised string
 * that can be displayed on the GUI.
 *
 * @param  type the type to translate.
 * @return a string with the localised name of the type.
 */
QString StoryItem::typeToString(ItemType type) {
    QString name = "";
    switch (type) {
        case StoryItem::Root:      name = ""; break;
        case StoryItem::Book:      name = tr("Book"); break;
        case StoryItem::Partition: name = tr("Partition"); break;
        case StoryItem::Chapter:   name = tr("Chapter"); break;
        case StoryItem::Scene:     name = tr("Scene"); break;
        case StoryItem::Page:      name = tr("Page"); break;
    }
    return name;
}

/**
 * Model Access
 * ============
 */

StoryItem *StoryItem::child(int row) {
    if (row < 0 || row >= m_childItems.size()) {
        return nullptr;
    } else {
        return m_childItems.at(row);
    }
}

int StoryItem::childCount() const {
    return m_childItems.count();
}

int StoryItem::row() const {
    if (m_parentItem) {
        return m_parentItem->m_childItems.indexOf(const_cast<StoryItem*>(this));
    } else {
        return 0;
    }
}

QVariant StoryItem::data() const {
    QVariantList itemData;
    itemData << m_name << m_wCount << typeToString(m_type);
    return QVariant::fromValue(itemData);
}

StoryItem *StoryItem::parentItem() {
    return m_parentItem;
}

} // namespace Collett
