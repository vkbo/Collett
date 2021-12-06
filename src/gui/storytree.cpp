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

    QMenu *scMenu = new QMenu(tr("Add Scene"));
    if (item->allowedChild(StoryItem::Scene)) {
        QAction *inAction = scMenu->addAction(tr("Inside"));
        connect(inAction, &QAction::triggered, [this, index]{doAddChild(index, StoryItem::Scene, GuiStoryTree::Inside);});
    }
    if (item->allowedSibling(StoryItem::Scene)) {
        QAction *bfAction = scMenu->addAction(tr("Before"));
        connect(bfAction, &QAction::triggered, [this, index]{doAddChild(index, StoryItem::Scene, GuiStoryTree::Before);});
        QAction *afAction = scMenu->addAction(tr("After"));
        connect(afAction, &QAction::triggered, [this, index]{doAddChild(index, StoryItem::Scene, GuiStoryTree::After);});
    }
    if (!scMenu->isEmpty()) {
        contextMenu.addMenu(scMenu);
    }

    QMenu *chMenu = new QMenu(tr("Add Chapter"));
    if (item->allowedChild(StoryItem::Chapter)) {
        QAction *inAction = chMenu->addAction(tr("Inside"));
        connect(inAction, &QAction::triggered, [this, index]{doAddChild(index, StoryItem::Chapter, GuiStoryTree::Inside);});
    }
    if (item->allowedSibling(StoryItem::Chapter)) {
        QAction *bfAction = chMenu->addAction(tr("Before"));
        connect(bfAction, &QAction::triggered, [this, index]{doAddChild(index, StoryItem::Chapter, GuiStoryTree::Before);});
        QAction *afAction = chMenu->addAction(tr("After"));
        connect(afAction, &QAction::triggered, [this, index]{doAddChild(index, StoryItem::Chapter, GuiStoryTree::After);});
    }
    if (!chMenu->isEmpty()) {
        contextMenu.addMenu(chMenu);
    }

    QMenu *ptMenu = new QMenu(tr("Add Partition"));
    if (item->allowedChild(StoryItem::Partition)) {
        QAction *inAction = ptMenu->addAction(tr("Inside"));
        connect(inAction, &QAction::triggered, [this, index]{doAddChild(index, StoryItem::Partition, GuiStoryTree::Inside);});
    }
    if (item->allowedSibling(StoryItem::Partition)) {
        QAction *bfAction = ptMenu->addAction(tr("Before"));
        connect(bfAction, &QAction::triggered, [this, index]{doAddChild(index, StoryItem::Partition, GuiStoryTree::Before);});
        QAction *afAction = ptMenu->addAction(tr("After"));
        connect(afAction, &QAction::triggered, [this, index]{doAddChild(index, StoryItem::Partition, GuiStoryTree::After);});
    }
    if (!ptMenu->isEmpty()) {
        contextMenu.addMenu(ptMenu);
    }

    QMenu *bkMenu = new QMenu(tr("Add Book"));
    if (item->allowedChild(StoryItem::Book)) {
        QAction *inAction = bkMenu->addAction(index.isValid() ? tr("Inside") : tr("Here"));
        connect(inAction, &QAction::triggered, [this, index]{doAddChild(index, StoryItem::Book, GuiStoryTree::Inside);});
    }
    if (item->allowedSibling(StoryItem::Book)) {
        QAction *bfAction = bkMenu->addAction(tr("Before"));
        connect(bfAction, &QAction::triggered, [this, index]{doAddChild(index, StoryItem::Book, GuiStoryTree::Before);});
        QAction *afAction = bkMenu->addAction(tr("After"));
        connect(afAction, &QAction::triggered, [this, index]{doAddChild(index, StoryItem::Book, GuiStoryTree::After);});
    }
    if (!bkMenu->isEmpty()) {
        contextMenu.addMenu(bkMenu);
    }

    QMenu *pgMenu = new QMenu(tr("Add Page"));
    if (item->allowedChild(StoryItem::Page)) {
        QAction *inAction = pgMenu->addAction(tr("Inside"));
        connect(inAction, &QAction::triggered, [this, index]{doAddChild(index, StoryItem::Page, GuiStoryTree::Inside);});
    }
    if (item->allowedSibling(StoryItem::Page)) {
        QAction *bfAction = pgMenu->addAction(tr("Before"));
        connect(bfAction, &QAction::triggered, [this, index]{doAddChild(index, StoryItem::Page, GuiStoryTree::Before);});
        QAction *afAction = pgMenu->addAction(tr("After"));
        connect(afAction, &QAction::triggered, [this, index]{doAddChild(index, StoryItem::Page, GuiStoryTree::After);});
    }
    if (!pgMenu->isEmpty()) {
        contextMenu.addMenu(pgMenu);
    }

    contextMenu.exec(QWidget::mapToGlobal(pos));
}

void GuiStoryTree::doAddChild(const QModelIndex &index, StoryItem::ItemType action, AddLocation loc) {
    qDebug() << action << loc;
}

} // namespace Collett
