/*
Collett – GUI Story Tree Class
==============================

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

#include "collett.h"
#include "storyitem.h"
#include "storytree.h"
#include "storymodel.h"
#include "storytreedelegate.h"

#include <QMenu>
#include <QPoint>
#include <QWidget>
#include <QTreeView>
#include <QModelIndex>

namespace Collett {

/**
 * Story Tree Contructor/Destructor
 * ================================
 */

GuiStoryTree::GuiStoryTree(QWidget *parent)
    : QTreeView(parent)
{
    this->setItemDelegate(new GuiStoryTreeDelegate(this));
    this->setHeaderHidden(true);
    this->setAlternatingRowColors(true);
    this->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(doOpenContextMenu(QPoint)));
}

/**
 * Class Setters
 * =============
 */

void GuiStoryTree::setTreeModel(StoryModel *model) {
    m_model = model;
    this->setModel(m_model);
}

/**
 * Class Private Slots
 * ===================
 */

void GuiStoryTree::doOpenContextMenu(const QPoint &pos) {

    QModelIndex index = this->indexAt(pos);
    StoryItem *item;
    if (index.isValid()) {
        item = static_cast<StoryItem*>(index.internalPointer());
    } else {
        item = m_model->rootItem();
    }
    if (!item) {
        return;
    }

    qDebug() << "Opening StoryTree context menu";

    QMenu contextMenu;

    QMenu *addChild = new QMenu(index.isValid() ? tr("Add Child Item") : tr("Append Item"));
    if (item->allowedChild(StoryItem::Book)) {
        QAction *newAction = addChild->addAction(tr("New Book"));
        connect(newAction, &QAction::triggered, [this, index]{doAddChild(index, StoryItem::Book);});
    }
    if (item->allowedChild(StoryItem::Partition)) {
        QAction *newAction = addChild->addAction(tr("New Partition"));
        connect(newAction, &QAction::triggered, [this, index]{doAddChild(index, StoryItem::Partition);});
    }
    if (item->allowedChild(StoryItem::Chapter)) {
        QAction *newAction = addChild->addAction(tr("New Chapter"));
        connect(newAction, &QAction::triggered, [this, index]{doAddChild(index, StoryItem::Chapter);});
    }
    if (item->allowedChild(StoryItem::Scene)) {
        QAction *newAction = addChild->addAction(tr("New Scene"));
        connect(newAction, &QAction::triggered, [this, index]{doAddChild(index, StoryItem::Scene);});
    }
    if (item->allowedChild(StoryItem::Page)) {
        QAction *newAction = addChild->addAction(tr("New Page"));
        connect(newAction, &QAction::triggered, [this, index]{doAddChild(index, StoryItem::Page);});
    }
    if (!addChild->isEmpty()) {
        contextMenu.addMenu(addChild);
    }

    contextMenu.exec(QWidget::mapToGlobal(pos));
}

void GuiStoryTree::doAddChild(const QModelIndex &index, int type) {
    qDebug() << type;
}

} // namespace Collett
