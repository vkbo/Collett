/*
** Collett – Project Item Model Class
** ==================================
**
** This file is a part of Collett
** Copyright 2020–2023, Veronica Berglyd Olsen
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

#include "itemmodel.h"
#include "item.h"
#include "icons.h"

#include <QUuid>
#include <QObject>
#include <QString>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QModelIndex>
#include <QLatin1String>
#include <QAbstractItemModel>

namespace Collett {

/**!
 * @brief Construct a new Item Model object.
 *
 * @param parent the parent object.
 */
ItemModel::ItemModel(QObject *parent) : QAbstractItemModel(parent) {}

ItemModel::~ItemModel() {
    qDebug() << "Destructor: ItemModel";
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
 * the @sa Item::toJsonObject function, which will build the entire tree
 * recursively.
 *
 * @return a JSON object.
 */
QJsonObject ItemModel::toJsonObject() {
    QJsonObject json = m_rootItem->toJsonObject();
    json[QLatin1String("c:format")] = "collett:itemmodel";
    return json;
}

/**!
 * @brief Load story model from JSON object.
 *
 * This function loads the root element data, validates it, and calls the
 * addChild method of Item on each child element defined within it.
 *
 * @param json the JSON object to be loaded into the model.
 * @return true if data was loaded successfully, false if an error occurred.
 */
bool ItemModel::fromJsonObject(const QJsonObject &json) {

    if (json.isEmpty()) {
        qWarning() << "Item Root: No data in JSON object";
        m_lastError = "Item Root: No data in JSON object";
        return false;
    }
    if (!json.contains(QLatin1String("c:format"))) {
        qWarning() << "Item Root: Not a valid item model JSON object";
        m_lastError = "Item Root: Not a valid item model JSON object";
        return false;
    }

    m_rootItem = new Item(QUuid(), "Hidden Root", Item::T_Hidden, Item::C_Hidden);

    if (!json.contains(QLatin1String("x:items"))) {
        qDebug() << "Item Root: No items found in JSON object";
        return true;
    }
    if (!json.value(QLatin1String("x:items")).isArray()) {
        qWarning() << "Item Root: x:items value is not a JSON array";
        m_lastError = "Item Root: x:items value is not a JSON array";
        return false;
    }

    emit beginResetModel();
    for (const QJsonValue &value : json[QLatin1String("x:items")].toArray()) {
        if (value.isObject()) {
            m_rootItem->addChild(value.toObject());
        } else {
            qWarning() << "Item Root: Child item is not a JSON object";
        }
    }
    emit endResetModel();

    return true;
}

/**!
 * @brief Create and add a new child item.
 *
 * Add a new item relative to the item @a relativeTo, with type @a type and a
 * location @a loc. The location can be Inside, in which case it is appended to
 * the end of the list of @a relativeTo child items; or Before or After, in which
 * case it is added directly above or below the @a relativeTo item.
 *
 * @param relative the item to add a new item relative to.
 * @param type     the type of the new item.
 * @param loc      the relative location of where to add the new item.
 * @return true if the item was successfully added, otherwise false.
 */
bool ItemModel::addItem(const QModelIndex &relative, Item::ItemType type, AddLocation loc) {

    QString name = tr("New %1").arg(Item::typeToLabel(type));

    if (!relative.isValid()) {

        int pos = m_rootItem->childCount();
        emit beginInsertRows(this->index(0, 0), pos, pos);
        Item *item = m_rootItem->addChild(name, type, pos);
        emit endInsertRows();
        qDebug() << "Added root item with name:" << name;
        return item != nullptr;

    } else {

        QModelIndex index = relative;

        Item *target = itemFromIndex(relative);
        if (!target) {
            return false;
        }

        int pos = target->childCount();
        if (loc == AddLocation::Before || loc == AddLocation::After) {
            target = target->parentItem();
            index = this->parent(relative);
            pos = target->row() + (loc == AddLocation::After ? 1 : 0);
        }
        if (!target) {
            return false;
        }

        emit beginInsertRows(index, pos, pos);
        Item *item = target->addChild(name, type, pos);
        emit endInsertRows();

        qDebug() << "Added item with name:" << name;
        return item != nullptr;

    }
}

/**!
 * @brief Check if the model is empty.
 *
 * @return true if there is no root item, otherwise false
 */
bool ItemModel::isEmpty() const {
    return m_rootItem == nullptr;
}

/**
 * Class Getters
 * =============
 */

Item *ItemModel::rootItem() const {
    return m_rootItem;
}

Item *ItemModel::itemFromIndex(const QModelIndex &index) {
    if (index.isValid()) {
        return static_cast<Item*>(index.internalPointer());
    } else {
        return nullptr;
    }
}

Item *ItemModel::itemFromHandle(const QUuid &uuid) {
    if (!uuid.isNull()) {
        return m_rootItem->findItemFromHandle(uuid);
    } else {
        return nullptr;
    }
}

QString ItemModel::itemName(const QModelIndex &index) {
    if (index.isValid()) {
        Item *item = static_cast<Item*>(index.internalPointer());
        return item->name();
    } else {
        return "";
    }
}

bool ItemModel::isExpanded(const QModelIndex &index) {
    if (index.isValid()) {
        Item *item = static_cast<Item*>(index.internalPointer());
        return item->isExpanded();
    } else {
        return false;
    }
}

/**
 * Model Edit
 * ==========
 */

void ItemModel::setItemName(const QModelIndex &index, const QString &name) {
    if (index.isValid()) {
        Item *item = static_cast<Item*>(index.internalPointer());
        item->setName(name);
    }
}

void ItemModel::setExpanded(const QModelIndex &index, bool state) {
    if (index.isValid()) {
        Item *item = static_cast<Item*>(index.internalPointer());
        item->setExpanded(state);
    }
}

/**
 * Model Access
 * ============
 */

QModelIndex ItemModel::index(int row, int column, const QModelIndex &parent) const {

    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    Item *parentItem;
    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = static_cast<Item*>(parent.internalPointer());
    }

    Item *childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    } else {
        return QModelIndex();
    }
}

QModelIndex ItemModel::parent(const QModelIndex &index) const {

    if (!index.isValid()) {
        return QModelIndex();
    }

    Item *childItem = static_cast<Item*>(index.internalPointer());
    Item *parentItem = childItem->parentItem();

    if (parentItem == m_rootItem) {
        return QModelIndex();
    } else {
        return createIndex(parentItem->row(), 0, parentItem);
    }
}

int ItemModel::rowCount(const QModelIndex &parent) const {

    Item *parentItem;
    if (parent.column() > 0) {
        return 0;
    }

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = static_cast<Item*>(parent.internalPointer());
    }

    return parentItem->childCount();
}

int ItemModel::columnCount(const QModelIndex &parent) const {
    return 2;
}

QVariant ItemModel::data(const QModelIndex &index, int role) const {

    if (!index.isValid()) {
        return QVariant();
    }

    Item *item = static_cast<Item*>(index.internalPointer());
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: // Title
                return QVariant::fromValue(item->name());
            case 1: // Word Count
                return QVariant::fromValue(QLocale().toString(item->wordCount()));
            default:
                return QVariant();
        }
    } else if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
            case 0: // Title
                return QVariant::fromValue(Qt::AlignLeading);
            case 1: // Word Count
                return QVariant::fromValue(Qt::AlignTrailing);
            default:
                return QVariant::fromValue(Qt::AlignLeading);
        }
    } else if (role == Qt::DecorationRole) {
        CollettIcons *icons = CollettIcons::instance();
        if (index.column() == 0) {
            switch (item->type()) {
                case Item::T_Root:
                    return icons->icon("projectRoot");
                case Item::T_Folder:
                    return icons->icon("projectFolder");
                case Item::T_Document:
                    return icons->icon("projectDocument");
                default:
                    return QVariant();
            }
        } else {
            return QVariant();
        }
    }

    return QVariant();
}

Qt::ItemFlags ItemModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    } else {
        return QAbstractItemModel::flags(index);
    }
}

QVariant ItemModel::headerData(int section, Qt::Orientation orientation, int role) const {
    return QVariant();
}

} // namespace Collett
