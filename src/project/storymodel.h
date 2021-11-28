/*
Collett – Project Story Model Class
===================================

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

#ifndef COLLETT_STORYMODEL_H
#define COLLETT_STORYMODEL_H

#include "storyitem.h"

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QModelIndex>
#include <QAbstractItemModel>

namespace Collett {

class StoryModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit StoryModel(QObject *parent=nullptr);
    ~StoryModel();

    // Class Methods
    QJsonObject toJsonObject();

    // Model Access
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    StoryItem *m_rootItem;

};
} // namespace Collett

#endif // COLLETT_STORYMODEL_H
