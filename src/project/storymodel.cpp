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
#include <QAbstractItemModel>

namespace Collett {

/*
    CollettStoryModel
    -----------------
    The data model of the project's stroy content.
    Example: https://doc.qt.io/qt-5/qtwidgets-itemviews-simpletreemodel-example.html
*/

CollettStoryModel::CollettStoryModel(const QString &data, QObject *parent)
    : QAbstractItemModel(parent)
{
    rootItem = new CollettStoryItem({tr("Title"), tr("Words")});
    rootItem->appendChild(new CollettStoryItem({"Title Page", 100}, rootItem));
    rootItem->appendChild(new CollettStoryItem({"Chapter 1", 150}, rootItem));
}

CollettStoryModel::~CollettStoryModel() {
    delete rootItem;
}

QModelIndex CollettStoryModel::index(int row, int column, const QModelIndex &parent) const {

    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    CollettStoryItem *parentItem;
    if (!parent.isValid()) {
        parentItem = rootItem;
    } else {
        parentItem = static_cast<CollettStoryItem*>(parent.internalPointer());
    }

    CollettStoryItem *childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    } else {
        return QModelIndex();
    }
}

QModelIndex CollettStoryModel::parent(const QModelIndex &index) const {

    if (!index.isValid()) {
        return QModelIndex();
    }

    CollettStoryItem *childItem = static_cast<CollettStoryItem*>(index.internalPointer());
    CollettStoryItem *parentItem = childItem->parentItem();

    if (parentItem == rootItem) {
        return QModelIndex();
    } else {
        return createIndex(parentItem->row(), 0, parentItem);
    }
}

int CollettStoryModel::rowCount(const QModelIndex &parent) const {

    CollettStoryItem *parentItem;
    if (parent.column() > 0) {
        return 0;
    }

    if (!parent.isValid()) {
        parentItem = rootItem;
    } else {
        parentItem = static_cast<CollettStoryItem*>(parent.internalPointer());
    }

    return parentItem->childCount();
}

int CollettStoryModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return static_cast<CollettStoryItem*>(parent.internalPointer())->columnCount();
    } else {
        return rootItem->columnCount();
    }
}

QVariant CollettStoryModel::data(const QModelIndex &index, int role) const {

    if (!index.isValid()) {
        return QVariant();
    }
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    CollettStoryItem *item = static_cast<CollettStoryItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags CollettStoryModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    } else {
        return QAbstractItemModel::flags(index);
    }
}

QVariant CollettStoryModel::headerData(int section, Qt::Orientation orientation, int role) const {

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return rootItem->data(section);
    } else {
        return QVariant();
    }
}

} // namespace Collett
