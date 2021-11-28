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

#include "storyitem.h"

#include <QVector>
#include <QVariant>

namespace Collett {

StoryItem::StoryItem(const QVector<QVariant> &data, StoryItem *parent)
    : m_itemData(data), m_parentItem(parent)
{}

StoryItem::~StoryItem() {
    qDeleteAll(m_childItems);
}

void StoryItem::appendChild(StoryItem *item) {
    m_childItems.append(item);
}

StoryItem *StoryItem::child(int row) {
    if (row < 0 || row >= m_childItems.size()) {
        return nullptr;
    }
    return m_childItems.at(row);
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
    return m_itemData.count();
}

QVariant StoryItem::data(int column) const {
    if (column < 0 || column >= m_itemData.size()) {
        return QVariant();
    } else {
        return m_itemData.at(column);
    }
}

StoryItem *StoryItem::parentItem() {
    return m_parentItem;
}

} // namespace Collett
