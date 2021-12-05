/*
Collett – Project Model Class
=============================

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

#include "storymodel.h"
#include "storyitem.h"

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QModelIndex>
#include <QAbstractItemModel>

namespace Collett {

/**
 * StoryModel
 * ==========
 * The data model of the project's stroy content.
 * Example: https://doc.qt.io/qt-5/qtwidgets-itemviews-simpletreemodel-example.html
*/

StoryModel::StoryModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_rootItem = new StoryItem("Story", StoryItem::Root);
    m_rootItem->appendChild(new StoryItem("Title Page", StoryItem::Page, m_rootItem));
    m_rootItem->appendChild(new StoryItem("Chapter 1", StoryItem::Chapter, m_rootItem));
    m_rootItem->appendChild(new StoryItem("Chapter 2", StoryItem::Chapter, m_rootItem));
    m_rootItem->appendChild(new StoryItem("Very long title on this element here", StoryItem::Page, m_rootItem));
    m_rootItem->child(1)->appendChild(new StoryItem("Scene 1.1", StoryItem::Scene, m_rootItem->child(1)));
    m_rootItem->child(1)->appendChild(new StoryItem("Scene 1.2", StoryItem::Scene, m_rootItem->child(1)));
    m_rootItem->child(2)->appendChild(new StoryItem("Scene 2.1", StoryItem::Scene, m_rootItem->child(2)));
    m_rootItem->child(2)->appendChild(new StoryItem("Scene 2.2", StoryItem::Scene, m_rootItem->child(2)));
}

StoryModel::~StoryModel() {
    delete m_rootItem;
}

/**
 * Class Getters
 * =============
 */

StoryItem *StoryModel::rootItem() const {
    return m_rootItem;
}

/**
 * Class Methods
 * =============
 */

QJsonObject StoryModel::toJsonObject() {
    return m_rootItem->toJsonObject();
}

/**
 * Model Access
 * ============
 */

QModelIndex StoryModel::index(int row, int column, const QModelIndex &parent) const {

    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    StoryItem *parentItem;
    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = static_cast<StoryItem*>(parent.internalPointer());
    }

    StoryItem *childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    } else {
        return QModelIndex();
    }
}

QModelIndex StoryModel::parent(const QModelIndex &index) const {

    if (!index.isValid()) {
        return QModelIndex();
    }

    StoryItem *childItem = static_cast<StoryItem*>(index.internalPointer());
    StoryItem *parentItem = childItem->parentItem();

    if (parentItem == m_rootItem) {
        return QModelIndex();
    } else {
        return createIndex(parentItem->row(), 0, parentItem);
    }
}

int StoryModel::rowCount(const QModelIndex &parent) const {

    StoryItem *parentItem;
    if (parent.column() > 0) {
        return 0;
    }

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = static_cast<StoryItem*>(parent.internalPointer());
    }

    return parentItem->childCount();
}

int StoryModel::columnCount(const QModelIndex &parent) const {
    return 1;
}

QVariant StoryModel::data(const QModelIndex &index, int role) const {

    if (!index.isValid()) {
        return QVariant();
    }
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    StoryItem *item = static_cast<StoryItem*>(index.internalPointer());

    return item->data();
}

Qt::ItemFlags StoryModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    } else {
        return QAbstractItemModel::flags(index);
    }
}

QVariant StoryModel::headerData(int section, Qt::Orientation orientation, int role) const {
    return QVariant();
}

} // namespace Collett
