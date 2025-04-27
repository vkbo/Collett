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
#include <QMimeData>
#include <QModelIndex>
#include <QString>
#include <QStringList>
#include <QUuid>

namespace Collett {

class Tree;
class ProjectModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit ProjectModel(Tree *parent=nullptr);
    ~ProjectModel();

    // Getters
    Node *invisibleRoot() const {return m_root;};
    Node *rootNode(Node *node);

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
    QModelIndex indexFromHandle(const QUuid &uuid);

    // Model Edit
    void  insertChild(Node *child, const QModelIndex &parent, qsizetype pos = -1);
    Node *removeChild(const QModelIndex &parent, qsizetype pos);
    void  multiMove(const QModelIndexList &indexes, const QModelIndex &parent, qsizetype pos = -1);

    Node *addRoot(QString name, ItemClass itemClass, const QModelIndex &selected);
    Node *addFolder(QString name, const QModelIndex &selected);
    Node *addFile(QString name, ItemLevel itemLevel, const QModelIndex &selected);

    // Drag and Drop
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    Qt::DropActions supportedDropActions() const;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

    // Static Methods
    static QList<QUuid> decodeMimeHandles(const QMimeData *mimeData);

private:
    Node *m_root = nullptr;
    Tree *m_tree = nullptr;

};
} // namespace Collett

#endif // COLLETT_PROJECT_MODEL_H
