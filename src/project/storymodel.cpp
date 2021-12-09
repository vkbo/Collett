/*
** Collett – Project Model Class
** =============================
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

#include "storymodel.h"
#include "storyitem.h"

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QModelIndex>
#include <QAbstractItemModel>

namespace Collett {

/**!
 * @brief Construct a new Story Model object.
 *
 * The data model of the project's story content.
 * Example: https://doc.qt.io/qt-5/qtwidgets-itemviews-simpletreemodel-example.html
 *
 * @param parent the parent object.
 */

StoryModel::StoryModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_rootItem = new StoryItem("Root", StoryItem::Root);

    StoryItem *bookItem = m_rootItem->addChild("Novel", StoryItem::Book);
    StoryItem *chp1Item = bookItem->addChild("Chapter 1", StoryItem::Chapter);
    StoryItem *chp2Item = bookItem->addChild("Chapter 2", StoryItem::Chapter);

    bookItem->addChild("Title Page", StoryItem::Page, 0);
    bookItem->addChild("Very long title on this element here", StoryItem::Page);

    chp1Item->addChild("Scene 1.1", StoryItem::Scene);
    chp1Item->addChild("Scene 1.2", StoryItem::Scene);
    chp2Item->addChild("Scene 2.1", StoryItem::Scene);
    chp2Item->addChild("Scene 2.2", StoryItem::Scene);
}

StoryModel::~StoryModel() {
    delete m_rootItem;
}

/**
 * Class Methods
 * =============
 */

/**!
 * @brief Build a JSON object of the model.
 *
 * Collect the story tree into a nested JSON object. This is a wrapper around
 * @sa StoryItem::toJsonObject function, which will build the entire tree
 * recursively.
 *
 * @return a JSON object.
 */
QJsonObject StoryModel::toJsonObject() {
    return m_rootItem->toJsonObject();
}

/**!
 * @brief Create and add a new child item.
 *
 * Add a new item relative to the item @a relativeTo, with type @a type and a
 * location @a loc. The location can be Inside, in which case it is appended to
 * the end of the list of @a relativeTo child items; or Before or After, in which
 * case it is added directly above or below the @a relativeTo item.
 *
 * @param relativeTo the item to add a new item relative to.
 * @param type       the type of the new item.
 * @param loc        the relative location of where to add the new item.
 * @return true if the item was successfully added, otherwise false.
 */
bool StoryModel::addItem(StoryItem *relativeTo, StoryItem::ItemType type, AddLocation loc) {
    if (!relativeTo) {
        return false;
    }
    StoryItem *target = relativeTo;
    int pos = relativeTo->childCount();
    if (loc == AddLocation::Before || loc == AddLocation::After) {
        target = relativeTo->parentItem();
        pos = relativeTo->row() + (loc == AddLocation::After ? 1 : 0);
    }
    if (!target) {
        return false;
    }
    qDebug() << target->row() << pos;
    emit beginInsertRows(createIndex(target->row(), 0, target), pos, pos);
    StoryItem *item = target->addChild(tr("New %1").arg(StoryItem::typeToString(type)), type, pos);
    emit endInsertRows();
    return item != nullptr;
}

/**
 * Class Getters
 * =============
 */

StoryItem *StoryModel::rootItem() const {
    return m_rootItem;
}

StoryItem *StoryModel::storyItem(const QModelIndex &index) {
    if (index.isValid()) {
        return static_cast<StoryItem*>(index.internalPointer());
    } else {
        return nullptr;
    }
}

QString StoryModel::itemName(const QModelIndex &index) {
    if (index.isValid()) {
        StoryItem *item = static_cast<StoryItem*>(index.internalPointer());
        return item->name();
    } else {
        return "";
    }
}

/**
 * Model Edit
 * ==========
 */

void StoryModel::setItemName(const QModelIndex &index, const QString &name) {
    if (index.isValid()) {
        StoryItem *item = static_cast<StoryItem*>(index.internalPointer());
        item->setName(name);
    }
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
