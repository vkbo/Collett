/*
** Collett – GUI Item Tree Class
** =============================
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

#ifndef GUI_ITEMTREE_H
#define GUI_ITEMTREE_H

#include "item.h"
#include "itemmodel.h"

#include <QPoint>
#include <QAction>
#include <QObject>
#include <QWidget>
#include <QTreeView>
#include <QModelIndex>

namespace Collett {

class GuiItemTree : public QTreeView
{
    Q_OBJECT

public:
    enum AddLocation{Before, After, Inside};

    GuiItemTree(QWidget *parent=nullptr);
    ~GuiItemTree() {};

    // Class Setters

    void setTreeModel(ItemModel *model);

    // Class Getters

    QModelIndex firstSelectedIndex();
    void getAllChildren(const QModelIndex &index, QModelIndexList &children);

public slots:
    void doEditName(bool checked);

private slots:
    void doOpenContextMenu(const QPoint &pos);
    void doAddChild(const QModelIndex &index, Item::ItemType type, ItemModel::AddLocation loc);
    void saveExpanded(const QModelIndex &index);
    void saveCollapsed(const QModelIndex &index);

private:
    ItemModel *m_model = nullptr;

    QAction *m_editItem;

};
} // namespace Collett

#endif // GUI_ITEMTREE_H
