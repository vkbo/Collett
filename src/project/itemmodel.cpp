/*
** Collett – Project Item Model Class
** ==================================
**
** This file is a part of Collett
** Copyright 2020–2022, Veronica Berglyd Olsen
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
#include <QModelIndexList>
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
ItemModel::ItemModel(QObject *parent) : QAbstractItemModel(parent) {
    m_type = ItemModel::Invalid;
    this->setModelName("");
    this->setModelIcon("");
}

ItemModel::ItemModel(ModelType type, QString name, QObject *parent) : QAbstractItemModel(parent) {
    m_type = type;
    m_rootItem = new Item(QUuid(), "Root", type == ItemModel::Story, Item::Root);
    this->setModelName(name);
    this->setModelIcon("");
}

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
 * @sa Item::toJsonObject function, which will build the entire tree
 * recursively.
 *
 * @return a JSON object.
 */
QJsonObject ItemModel::toJsonObject() {

    QJsonObject json = m_rootItem->toJsonObject();
    QString modelType = this->modelTypeToString(m_type);

    if (!modelType.isEmpty()) {
        json[QLatin1String("c:model")] = modelType;
        json[QLatin1String("u:name")] = m_name;
        json[QLatin1String("u:icon")] = m_icon;
        return json;
    } else {
        return QJsonObject();
    }
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
    if (!json.contains(QLatin1String("u:type")) || !json.contains(QLatin1String("c:model"))) {
        qWarning() << "Item Root: Not a valid story item JSON object";
        m_lastError = "Item Root: Not a valid story item JSON object";
        return false;
    }

    Item::ItemType type = Item::typeFromString(json.value(QLatin1String("u:type")).toString());
    if (type != Item::Root) {
        qWarning() << "Item Root: No ROOT item found in JSON object";
        m_lastError = "Item Root: No ROOT item found in JSON object";
        return false;
    }

    m_type = ItemModel::modelTypeFromString(json.value(QLatin1String("c:model")).toString());
    m_rootItem = new Item(QUuid(), "Root", m_type == ItemModel::Story, Item::Root);

    if (json.contains(QLatin1String("u:name"))) {
        this->setModelName(json.value(QLatin1String("u:name")).toString());
    }

    if (json.contains(QLatin1String("u:icon"))) {
        this->setModelIcon(json.value(QLatin1String("u:icon")).toString());
    }

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
 * @param relativeTo the item to add a new item relative to.
 * @param type       the type of the new item.
 * @param loc        the relative location of where to add the new item.
 * @return true if the item was successfully added, otherwise false.
 */
bool ItemModel::addItem(Item *relativeTo, Item::ItemType type, AddLocation loc) {
    if (!relativeTo) {
        return false;
    }
    Item *target = relativeTo;
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
    Item *item = target->addChild(tr("New %1").arg(Item::typeToLabel(type)), type, pos);
    emit endInsertRows();
    return item != nullptr;
}

/**!
 * @brief Check if the model is empty.
 *
 * @return true if there is no root item, otherwise false
 */
bool ItemModel::isEmpty() const {
    return m_rootItem == nullptr;
}

/**!
 * @brief Check if the model is valid.
 *
 * @return false if the model type is Invalid, otherwise true
 */
bool ItemModel::isValid() const {
    return m_type != ItemModel::Invalid;
}

/**
 * Class Getters
 * =============
 */

Item *ItemModel::rootItem() const {
    return m_rootItem;
}

Item *ItemModel::storyItem(const QModelIndex &index) {
    if (index.isValid()) {
        return static_cast<Item*>(index.internalPointer());
    } else {
        return nullptr;
    }
}

QUuid ItemModel::itemHandle(const QModelIndex &index) {
    if (index.isValid()) {
        Item *item = static_cast<Item*>(index.internalPointer());
        return item->handle();
    } else {
        return QUuid();
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
 * Static Methods
 * ==============
 */

QString ItemModel::modelTypeToLabel(ModelType type) {
    QString name = "";
    switch (type) {
        case ItemModel::Invalid:    name = ""; break;
        case ItemModel::Story:      name = tr("Story"); break;
        case ItemModel::Plot:       name = tr("Plot"); break;
        case ItemModel::Characters: name = tr("Characters"); break;
        case ItemModel::Locations:  name = tr("Locations"); break;
    }
    return name;
}

QString ItemModel::modelTypeToIcon(ModelType type) {
    QString icon = "genericModel";
    switch (type) {
        case ItemModel::Invalid:    icon = "genericModel"; break;
        case ItemModel::Story:      icon = "storyModel"; break;
        case ItemModel::Plot:       icon = "plotModel"; break;
        case ItemModel::Characters: icon = "characterModel"; break;
        case ItemModel::Locations:  icon = "locationModel"; break;
    }
    return icon;
}

QString ItemModel::modelTypeToString(ModelType type) {
    QString name = "";
    switch (type) {
        case ItemModel::Invalid:    name = ""; break;
        case ItemModel::Story:      name = "STORY"; break;
        case ItemModel::Plot:       name = "PLOT"; break;
        case ItemModel::Characters: name = "CHARACTERS"; break;
        case ItemModel::Locations:  name = "LOCATIONS"; break;
    }
    return name;
}

ItemModel::ModelType ItemModel::modelTypeFromString(const QString &value) {
    QString upper = value.toUpper();
    if (upper == "STORY") {
        return ItemModel::Story;
    } else if (upper == "PLOT") {
        return ItemModel::Plot;
    } else if (upper == "CHARACTERS") {
        return ItemModel::Characters;
    } else if (upper == "LOCATIONS") {
        return ItemModel::Locations;
    } else {
        return ItemModel::Invalid;
    }
}

/**
 * Model Edit
 * ==========
 */

void ItemModel::setModelName(const QString &name) {
    m_name = name.simplified();
    if (m_name.isEmpty()) {
        m_name = tr("No Name");
    }
}

void ItemModel::setModelIcon(const QString &icon) {
    CollettIcons *icons = CollettIcons::instance();
    if (icons->contains(icon)) {
        m_icon = icon;
    } else {
        m_icon = ItemModel::modelTypeToIcon(m_type);
    }
}

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
    return 1;
}

QVariant ItemModel::data(const QModelIndex &index, int role) const {

    if (!index.isValid()) {
        return QVariant();
    }
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    Item *item = static_cast<Item*>(index.internalPointer());

    return item->data();
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
