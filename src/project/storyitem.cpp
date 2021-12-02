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

namespace Collett {

StoryItem::StoryItem(const QString &label, StoryItem *parent)
    : m_parentItem(parent)
{
    m_handle = QUuid::createUuid();
    m_label  = label;
    m_wCount = 0;
}

StoryItem::~StoryItem() {
    qDeleteAll(m_childItems);
}

/*
    Item Structure
    ==============
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

    if (!m_parentItem) {
        item["handle"] = "ROOT";
        item["xItems"] = children;
    } else {
        item["handle"] = m_handle.toString(QUuid::WithoutBraces);
        item["label"]  = m_label;
        item["order"]  = row();
        item["wCount"] = m_wCount;
        if (children.size() > 0) {
            item["xItems"] = children;
        }
    }

    return item;
}

/*
    Setters
    =======
*/

void StoryItem::setLabel(const QString &label) {
    m_label = label;
}


void StoryItem::setWordCount(int count) {
    m_wCount = count > 0 ? count : 0;
}

/*
    Getters
    =======
*/

int StoryItem::wordCount() {
    return m_wCount;
}

int StoryItem::childWordCounts() {
    int tCount = 0;
    for (StoryItem* child : m_childItems) {
        tCount += child->childWordCounts();
    }
    return tCount;
}

/*
    Model Access
    ============
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

int StoryItem::columnCount() const {
    return COL_STORY_TREE_COL_COUNT;
}

QVariant StoryItem::data(int column) const {
    switch (column) {
    case 0:
        return QVariant(m_label);
        break;

    case 1:
        return QVariant(m_wCount);
        break;
    
    default:
        return QVariant();
        break;
    }
}

StoryItem *StoryItem::parentItem() {
    return m_parentItem;
}

} // namespace Collett
