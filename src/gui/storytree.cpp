/*
** Collett – GUI Story Tree Class
** ==============================
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

#include "collett.h"
#include "storyitem.h"
#include "storytree.h"
#include "storymodel.h"
#include "storytreedelegate.h"

#include <QMenu>
#include <QPoint>
#include <QAction>
#include <QWidget>
#include <QTreeView>
#include <QModelIndex>
#include <QInputDialog>
#include <QKeySequence>

namespace Collett {

/**
 * Story Tree Contructor/Destructor
 * ================================
 */

/**!
 * @brief Construct a new Gui Story Tree object
 *
 * @param parent the parent widget.
 */
GuiStoryTree::GuiStoryTree(QWidget *parent)
    : QTreeView(parent)
{
    this->setItemDelegate(new GuiStoryTreeDelegate(this));
    this->setHeaderHidden(true);
    this->setAlternatingRowColors(true);

    // Item Actions
    m_editItem = new QAction(tr("Rename"), this);
    m_editItem->setShortcut(QKeySequence("F2"));
    connect(m_editItem, SIGNAL(triggered(bool)), this, SLOT(doEditName(bool)));
    this->addAction(m_editItem);

    // Connect the Context Menu
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

/**!
 * @brief Slot to open a context menu on the tree.
 *
 * The context menu is generated on the fly and opened at the position of the
 * cursor.
 *
 * @param pos the position of the cursor.
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

    // Item Options
    // ------------

    contextMenu.addAction(m_editItem);
    contextMenu.addSeparator();

    // New Items
    // ---------

    QMenu *scMenu = new QMenu(tr("Add Scene"));
    if (item->allowedChild(StoryItem::Scene)) {
        QAction *inAction = scMenu->addAction(tr("Inside"));
        connect(inAction, &QAction::triggered, [this, item]{doAddChild(item, StoryItem::Scene, StoryModel::Inside);});
    }
    if (item->allowedSibling(StoryItem::Scene)) {
        QAction *bfAction = scMenu->addAction(tr("Before"));
        connect(bfAction, &QAction::triggered, [this, item]{doAddChild(item, StoryItem::Scene, StoryModel::Before);});
        QAction *afAction = scMenu->addAction(tr("After"));
        connect(afAction, &QAction::triggered, [this, item]{doAddChild(item, StoryItem::Scene, StoryModel::After);});
    }
    if (!scMenu->isEmpty()) {
        contextMenu.addMenu(scMenu);
    }

    QMenu *chMenu = new QMenu(tr("Add Chapter"));
    if (item->allowedChild(StoryItem::Chapter)) {
        QAction *inAction = chMenu->addAction(tr("Inside"));
        connect(inAction, &QAction::triggered, [this, item]{doAddChild(item, StoryItem::Chapter, StoryModel::Inside);});
    }
    if (item->allowedSibling(StoryItem::Chapter)) {
        QAction *bfAction = chMenu->addAction(tr("Before"));
        connect(bfAction, &QAction::triggered, [this, item]{doAddChild(item, StoryItem::Chapter, StoryModel::Before);});
        QAction *afAction = chMenu->addAction(tr("After"));
        connect(afAction, &QAction::triggered, [this, item]{doAddChild(item, StoryItem::Chapter, StoryModel::After);});
    }
    if (!chMenu->isEmpty()) {
        contextMenu.addMenu(chMenu);
    }

    QMenu *ptMenu = new QMenu(tr("Add Partition"));
    if (item->allowedChild(StoryItem::Partition)) {
        QAction *inAction = ptMenu->addAction(tr("Inside"));
        connect(inAction, &QAction::triggered, [this, item]{doAddChild(item, StoryItem::Partition, StoryModel::Inside);});
    }
    if (item->allowedSibling(StoryItem::Partition)) {
        QAction *bfAction = ptMenu->addAction(tr("Before"));
        connect(bfAction, &QAction::triggered, [this, item]{doAddChild(item, StoryItem::Partition, StoryModel::Before);});
        QAction *afAction = ptMenu->addAction(tr("After"));
        connect(afAction, &QAction::triggered, [this, item]{doAddChild(item, StoryItem::Partition, StoryModel::After);});
    }
    if (!ptMenu->isEmpty()) {
        contextMenu.addMenu(ptMenu);
    }

    QMenu *bkMenu = new QMenu(tr("Add Book"));
    if (item->allowedChild(StoryItem::Book)) {
        QAction *inAction = bkMenu->addAction(index.isValid() ? tr("Inside") : tr("Here"));
        connect(inAction, &QAction::triggered, [this, item]{doAddChild(item, StoryItem::Book, StoryModel::Inside);});
    }
    if (item->allowedSibling(StoryItem::Book)) {
        QAction *bfAction = bkMenu->addAction(tr("Before"));
        connect(bfAction, &QAction::triggered, [this, item]{doAddChild(item, StoryItem::Book, StoryModel::Before);});
        QAction *afAction = bkMenu->addAction(tr("After"));
        connect(afAction, &QAction::triggered, [this, item]{doAddChild(item, StoryItem::Book, StoryModel::After);});
    }
    if (!bkMenu->isEmpty()) {
        contextMenu.addMenu(bkMenu);
    }

    QMenu *pgMenu = new QMenu(tr("Add Page"));
    if (item->allowedChild(StoryItem::Page)) {
        QAction *inAction = pgMenu->addAction(tr("Inside"));
        connect(inAction, &QAction::triggered, [this, item]{doAddChild(item, StoryItem::Page, StoryModel::Inside);});
    }
    if (item->allowedSibling(StoryItem::Page)) {
        QAction *bfAction = pgMenu->addAction(tr("Before"));
        connect(bfAction, &QAction::triggered, [this, item]{doAddChild(item, StoryItem::Page, StoryModel::Before);});
        QAction *afAction = pgMenu->addAction(tr("After"));
        connect(afAction, &QAction::triggered, [this, item]{doAddChild(item, StoryItem::Page, StoryModel::After);});
    }
    if (!pgMenu->isEmpty()) {
        contextMenu.addMenu(pgMenu);
    }

    contextMenu.exec(QWidget::mapToGlobal(pos));
}

/**!
 * @brief Slot triggered by the rename option in the context menu.
 *
 * @param bool unused.
 */
void GuiStoryTree::doEditName(bool checked) {
    Q_UNUSED(checked);

    QModelIndexList sel = this->selectedIndexes();
    if (sel.size() < 1) {
        qDebug() << "No item selected";
        return;
    }

    QString oldName = m_model->itemName(sel.at(0));
    qDebug() << "Requested rename of item" << oldName;

    bool ok;
    QString newName = QInputDialog::getText(
        this, tr("Rename Story Item"), tr("New name:"), QLineEdit::Normal, oldName, &ok
    );
    if (ok && !newName.isEmpty()) {
        m_model->setItemName(sel.at(0), newName);
    }
}

/**!
 * @brief Slot triggered by the context menu on the tree.
 *
 * The slot will forward the call to create a new story item to the model.
 *
 * @param item the item to add a child relative to.
 * @param type the type of item to add.
 * @param loc  the relative location of where to add the new item.
 */
void GuiStoryTree::doAddChild(StoryItem *item, StoryItem::ItemType type, StoryModel::AddLocation loc) {
    if (m_model) {
        if (m_model->addItem(item, type, loc)) {
            qDebug() << "Added" << StoryItem::typeToString(type);
        } else {
            qWarning() << "Failed to add" << StoryItem::typeToString(type);
        }
    }
}

} // namespace Collett
