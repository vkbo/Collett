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

#ifndef COLLETT_ITEMMODEL_H
#define COLLETT_ITEMMODEL_H

#include "item.h"

#include <QUuid>
#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QModelIndex>
#include <QModelIndexList>
#include <QAbstractItemModel>

namespace Collett {

class ItemModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum ModelType{Invalid, Story, Plot, Characters, Locations};
    enum AddLocation{Before, After, Inside};

    explicit ItemModel(QObject *parent=nullptr);
    explicit ItemModel(ModelType type, QString name, QObject *parent=nullptr);
    ~ItemModel();

    // Class Methods

    QJsonObject toJsonObject();
    bool fromJsonObject(const QJsonObject &json);
    bool addItem(const QModelIndex &relative, Item::ItemType type, AddLocation loc);
    bool isEmpty() const;
    bool isValid() const;

    // Class Getters

    QString modelName() const;
    QString modelIcon() const;
    ModelType modelType() const;

    Item *rootItem() const;
    Item *itemFromIndex(const QModelIndex &index);
    Item *itemFromHandle(const QUuid &uuid);
    QString itemName(const QModelIndex &index);
    bool isExpanded(const QModelIndex &index);

    // Static Methods

    static QString modelTypeToLabel(ModelType type);
    static QString modelTypeToIcon(ModelType type);
    static QString modelTypeToString(ModelType type);
    static ModelType modelTypeFromString(const QString &value);

    // Model Edit

    void setModelName(const QString &name);
    void setModelIcon(const QString &icon);
    void setItemName(const QModelIndex &index, const QString &name);
    void setExpanded(const QModelIndex &index, bool state);

    // Model Access

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    ModelType m_type;
    QString   m_name;
    QString   m_icon;
    Item     *m_rootItem = nullptr;

    QString m_lastError = "";

};
} // namespace Collett

#endif // COLLETT_ITEMMODEL_H
