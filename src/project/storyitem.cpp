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
#include <QXmlStreamWriter>

namespace Collett {

/**!
 * @brief Construct a new StoryItem object.
 *
 * @param label  the label of the new item.
 * @param type   the type of the new item.
 * @param parent the parent of the new item, optional.
 */
StoryItem::StoryItem(const QUuid &uuid, const QString &name, ItemType type, StoryItem *parent)
    : m_parentItem(parent)
{
    m_childItems = QVector<StoryItem*>{};
    m_handle = uuid;
    m_name   = name.simplified();
    m_type   = type;
    m_wCount = 0;
}

StoryItem::~StoryItem() {
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
StoryItem *StoryItem::addChild(const QString &name, ItemType type, int pos) {
    if (!this->allowedChild(type)) {
        return nullptr;
    }

    StoryItem *item = new StoryItem(QUuid::createUuid(), name, type, this);
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
StoryItem *StoryItem::addChild(const QJsonObject &json) {

    if (json.isEmpty()) {
        return nullptr;
    }

    QUuid    handle = QUuid();
    QString  name   = "";
    ItemType type   = StoryItem::Invalid;
    int      wCount = 0;

    if (json.contains("handle")) {
        handle = QUuid(json["handle"].toString());
    }
    if (json.contains("name")) {
        name = json["name"].toString();
    }
    if (json.contains("type")) {
        type = StoryItem::typeFromString(json["type"].toString());
    }
    if (json.contains("wCount")) {
        wCount = json["wCount"].toInt();
    }

    if (type == StoryItem::Invalid) {
        qWarning() << "Invalid story item type encountered";
        return nullptr;
    }
    if (type == StoryItem::Root) {
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
    
    StoryItem *item = new StoryItem(handle, name, type, this);
    item->setWordCount(wCount);
    m_childItems.append(item);

    if (json.contains("xItems")) {
        if (json["xItems"].isArray()) {
            for (const QJsonValue &value : json["xItems"].toArray()) {
                if (value.isObject()) {
                    item->addChild(value.toObject());
                } else {
                    qWarning() << "StoryItem: Child item is not a JSON object";
                }
            }
        }
    }

    return item;
}

void StoryItem::writeXML(QXmlStreamWriter &xmlWriter) {

    if (!m_parentItem) {
        xmlWriter.writeStartElement(Collett::ColNsCollett, "tree");
        xmlWriter.writeAttribute(Collett::ColNsItem, "class", "story");

    } else {
        xmlWriter.writeStartElement(Collett::ColNsCollett, "item");

        switch (m_type) {
            case StoryItem::Root:
                xmlWriter.writeAttribute(Collett::ColNsItem, "type", "root");
                break;
            case StoryItem::Book:
                xmlWriter.writeAttribute(Collett::ColNsItem, "type", "book");
                break;
            case StoryItem::Partition:
                xmlWriter.writeAttribute(Collett::ColNsItem, "type", "partition");
                break;
            case StoryItem::Chapter:
                xmlWriter.writeAttribute(Collett::ColNsItem, "type", "chapter");
                break;
            case StoryItem::Scene:
                xmlWriter.writeAttribute(Collett::ColNsItem, "type", "scene");
                break;
            case StoryItem::Page:
                xmlWriter.writeAttribute(Collett::ColNsItem, "type", "page");
                break;
            default:
                return;
                break;
        }

        xmlWriter.writeAttribute(Collett::ColNsItem, "handle", m_handle.toString(QUuid::WithoutBraces));
        xmlWriter.writeAttribute(Collett::ColNsItem, "order", QString().setNum(row()));
        xmlWriter.writeAttribute(Collett::ColNsItem, "words", QString().setNum(m_wCount));

        xmlWriter.writeStartElement(Collett::ColNsItem, "name");
        xmlWriter.writeCharacters(m_name);
        xmlWriter.writeEndElement(); // name
    }

    for (qsizetype i=0; i<m_childItems.size(); ++i) {
        m_childItems.at(i)->writeXML(xmlWriter);
    }

    xmlWriter.writeEndElement(); // root or item
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
    m_name = name.simplified();
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
        case StoryItem::Invalid:   name = ""; break;
    }
    return name;
}

StoryItem::ItemType StoryItem::typeFromString(const QString &value) {
    QString upper = value.toUpper();
    if (upper == "ROOT") {
        return StoryItem::Root;
    } else if (upper == "BOOK") {
        return StoryItem::Book;
    } else if (upper == "PARTITION") {
        return StoryItem::Partition;
    } else if (upper == "CHAPTER") {
        return StoryItem::Chapter;
    } else if (upper == "SCENE") {
        return StoryItem::Scene;
    } else if (upper == "PAGE") {
        return StoryItem::Page;
    } else {
        return StoryItem::Invalid;
    }
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
