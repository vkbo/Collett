/*
Collett – Project Story Item Class
==================================

This file is a part of Collett
Copyright 2020–2021, Veronica Berglyd Olsen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
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

StoryItem::StoryItem(const QString &label, ItemType type, StoryItem *parent)
    : m_parentItem(parent)
{
    m_handle = QUuid::createUuid();
    m_label  = label;
    m_type   = type;
    m_wCount = 1234;
}

StoryItem::~StoryItem() {
    qDeleteAll(m_childItems);
}

/**
 * Item Structure
 * ==============
 */

void StoryItem::appendChild(StoryItem *item) {
    m_childItems.append(item);
}

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
        item["label"]  = m_label;
        item["type"]   = type;
        item["order"]  = row();
        item["wCount"] = m_wCount;
        if (children.size() > 0) {
            item["xItems"] = children;
        }
    }

    return item;
}

/**
 * Class Setters
 * =============
 */

void StoryItem::setLabel(const QString &label) {
    m_label = label;
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

QString StoryItem::localTypeName() const {
    QString type = "";
    switch (m_type) {
        case StoryItem::Root:      type = ""; break;
        case StoryItem::Book:      type = tr("Book"); break;
        case StoryItem::Partition: type = tr("Partition"); break;
        case StoryItem::Chapter:   type = tr("Chapter"); break;
        case StoryItem::Scene:     type = tr("Scene"); break;
        case StoryItem::Page:      type = tr("Page"); break;
    }
    return type;
}

/**
 * Class Methods
 * =============
 */

bool StoryItem::allowedChild(StoryItem::ItemType type) const {
    switch (m_type) {
        //   Map current item's type to incoming type       PSCPBR
        case StoryItem::ItemType::Root:      return bool(0b111110 & type); break;
        case StoryItem::ItemType::Book:      return bool(0b111100 & type); break;
        case StoryItem::ItemType::Partition: return bool(0b111000 & type); break;
        case StoryItem::ItemType::Chapter:   return bool(0b010000 & type); break;
        case StoryItem::ItemType::Scene:     return bool(0b000000 & type); break;
        case StoryItem::ItemType::Page:      return bool(0b000000 & type); break;
    }
    return false;
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
    itemData << m_label << m_wCount << localTypeName();
    return QVariant::fromValue(itemData);
}

StoryItem *StoryItem::parentItem() {
    return m_parentItem;
}

} // namespace Collett
