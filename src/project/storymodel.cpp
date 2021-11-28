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

/*
    StoryModel
    ----------
    The data model of the project's stroy content.
    Example: https://doc.qt.io/qt-5/qtwidgets-itemviews-simpletreemodel-example.html
*/

StoryModel::StoryModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_rootItem = new StoryItem("");
    m_rootItem->appendChild(new StoryItem("Title Page", m_rootItem));
    m_rootItem->appendChild(new StoryItem("Chapter 1", m_rootItem));
}

StoryModel::~StoryModel() {
    delete m_rootItem;
}

/*
    Class Methods
    =============
*/

QJsonObject StoryModel::toJsonObject() {
    return m_rootItem->toJsonObject();
}

/*
    Model Access
    ============
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
    if (parent.isValid()) {
        return static_cast<StoryItem*>(parent.internalPointer())->columnCount();
    } else {
        return m_rootItem->columnCount();
    }
}

QVariant StoryModel::data(const QModelIndex &index, int role) const {

    if (!index.isValid()) {
        return QVariant();
    }
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    StoryItem *item = static_cast<StoryItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags StoryModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    } else {
        return QAbstractItemModel::flags(index);
    }
}

QVariant StoryModel::headerData(int section, Qt::Orientation orientation, int role) const {

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return QVariant(tr("Label"));
            break;

        case 1:
            return QVariant(tr("Words"));
            break;
        
        default:
            return QVariant();
            break;
        }
    } else {
        return QVariant();
    }
}

} // namespace Collett
