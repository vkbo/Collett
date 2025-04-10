/*
** Collett – Project Model Class
** =============================
**
** This file is a part of Collett
** Copyright (C) 2025 Veronica Berglyd Olsen
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

#ifndef COLLETT_PROJECT_MODEL_H
#define COLLETT_PROJECT_MODEL_H

#include "collett.h"
#include "node.h"

#include <QAbstractItemModel>
#include <QJsonObject>
#include <QList>
#include <QModelIndex>
#include <QString>

namespace Collett {

class ProjectModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit ProjectModel(QObject *parent=nullptr);
    ~ProjectModel();

    // Getters
    Node *invisibleRoot() const {return m_root;};

    // Methods
    void pack(QJsonObject &data);
    void unpack(const QJsonObject &data);

    // Model Access
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QList<QModelIndex> allExpanded();
    Node *nodeAtIndex(const QModelIndex &index);

    // Model Edit
    void  insertChild(Node *child, const QModelIndex &parent, qsizetype pos = -1);
    Node *addRoot(QString name, ItemClass itemClass, const QModelIndex &relative);
    Node *addFolder(QString name, const QModelIndex &relative);
    Node *addFile(QString name, ItemLevel itemLevel, const QModelIndex &parent, qsizetype pos = -1);

private:
    Node *m_root = nullptr;

};
} // namespace Collett

#endif // COLLETT_PROJECT_MODEL_H
