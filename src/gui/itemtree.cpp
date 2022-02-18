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

#include "collett.h"
#include "item.h"
#include "itemtree.h"
#include "itemmodel.h"
#include "itemtreedelegate.h"

#include <QMenu>
#include <QPoint>
#include <QAction>
#include <QWidget>
#include <QTreeView>
#include <QModelIndex>
#include <QInputDialog>
#include <QKeySequence>
#include <QModelIndexList>

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
GuiItemTree::GuiItemTree(QWidget *parent)
    : QTreeView(parent)
{
    this->setItemDelegate(new GuiItemTreeDelegate(this));
    this->setHeaderHidden(true);
    this->setAlternatingRowColors(true);
    this->setExpandsOnDoubleClick(false);

    // Connect the Context Menu
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(doOpenContextMenu(QPoint)));
    connect(this, SIGNAL(expanded(const QModelIndex&)),
            this, SLOT(saveExpanded(const QModelIndex&)));
    connect(this, SIGNAL(collapsed(const QModelIndex&)),
            this, SLOT(saveCollapsed(const QModelIndex&)));
}

/**
 * Class Setters
 * =============
 */

void GuiItemTree::setTreeModel(ItemModel *model) {

    m_model = model;
    this->setModel(m_model);

    // Restore Expanded State
    QModelIndexList allChildren;
    this->getAllChildren(m_model->index(0, 0), allChildren);
    this->blockSignals(true);
    for (const QModelIndex &child : allChildren) {
        this->setExpanded(child, m_model->isExpanded(child));
    }
    this->blockSignals(false);
}

/**
 * Class Getters
 * =============
 */

QModelIndex GuiItemTree::firstSelectedIndex() {
    QModelIndexList selections = this->selectedIndexes();
    if (!selections.isEmpty()) {
        return selections.at(0);
    } else {
        return QModelIndex();
    }
}

void GuiItemTree::getAllChildren(const QModelIndex &index, QModelIndexList &children) {
    children.append(index);
    for (int i = 0; i < model()->rowCount(index); ++i) {
        getAllChildren(model()->index(i, 0, index), children);
    }
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
void GuiItemTree::doOpenContextMenu(const QPoint &pos) {

    QModelIndex index = this->indexAt(pos);
    Item *item;
    bool root = false;
    if (index.isValid()) {
        item = static_cast<Item*>(index.internalPointer());
    } else {
        item = m_model->rootItem();
        root = true;
    }
    if (!item) {
        return;
    }

    qDebug() << "Opening StoryTree context menu";

    QMenu contextMenu;

    // Item Options
    // ------------

    // contextMenu.addAction(m_editItem);
    // contextMenu.addSeparator();

    // New Items
    // ---------

    QMenu *scMenu = new QMenu(tr("Add Scene"));
    if (item->allowedChild(Item::Scene)) {
        QAction *inAction = scMenu->addAction(tr("Inside"));
        connect(inAction, &QAction::triggered, [this, index]{doAddChild(index, Item::Scene, ItemModel::Inside);});
    }
    if (item->allowedSibling(Item::Scene)) {
        QAction *bfAction = scMenu->addAction(tr("Before"));
        connect(bfAction, &QAction::triggered, [this, index]{doAddChild(index, Item::Scene, ItemModel::Before);});
        QAction *afAction = scMenu->addAction(tr("After"));
        connect(afAction, &QAction::triggered, [this, index]{doAddChild(index, Item::Scene, ItemModel::After);});
    }
    if (!scMenu->isEmpty()) {
        contextMenu.addMenu(scMenu);
    }

    QMenu *chMenu = new QMenu(tr("Add Chapter"));
    if (item->allowedChild(Item::Chapter)) {
        QAction *inAction = chMenu->addAction(tr("Inside"));
        connect(inAction, &QAction::triggered, [this, index]{doAddChild(index, Item::Chapter, ItemModel::Inside);});
    }
    if (item->allowedSibling(Item::Chapter)) {
        QAction *bfAction = chMenu->addAction(tr("Before"));
        connect(bfAction, &QAction::triggered, [this, index]{doAddChild(index, Item::Chapter, ItemModel::Before);});
        QAction *afAction = chMenu->addAction(tr("After"));
        connect(afAction, &QAction::triggered, [this, index]{doAddChild(index, Item::Chapter, ItemModel::After);});
    }
    if (!chMenu->isEmpty()) {
        contextMenu.addMenu(chMenu);
    }

    QMenu *ptMenu = new QMenu(tr("Add Partition"));
    if (item->allowedChild(Item::Partition)) {
        QAction *inAction = ptMenu->addAction(tr("Inside"));
        connect(inAction, &QAction::triggered, [this, index]{doAddChild(index, Item::Partition, ItemModel::Inside);});
    }
    if (item->allowedSibling(Item::Partition)) {
        QAction *bfAction = ptMenu->addAction(tr("Before"));
        connect(bfAction, &QAction::triggered, [this, index]{doAddChild(index, Item::Partition, ItemModel::Before);});
        QAction *afAction = ptMenu->addAction(tr("After"));
        connect(afAction, &QAction::triggered, [this, index]{doAddChild(index, Item::Partition, ItemModel::After);});
    }
    if (!ptMenu->isEmpty()) {
        contextMenu.addMenu(ptMenu);
    }

    QMenu *bkMenu = new QMenu(tr("Add Book"));
    if (item->allowedChild(Item::Book)) {
        QAction *inAction = bkMenu->addAction(root ? tr("Here") : tr("Inside"));
        connect(inAction, &QAction::triggered, [this, index]{doAddChild(index, Item::Book, ItemModel::Inside);});
    }
    if (item->allowedSibling(Item::Book)) {
        QAction *bfAction = bkMenu->addAction(tr("Before"));
        connect(bfAction, &QAction::triggered, [this, index]{doAddChild(index, Item::Book, ItemModel::Before);});
        QAction *afAction = bkMenu->addAction(tr("After"));
        connect(afAction, &QAction::triggered, [this, index]{doAddChild(index, Item::Book, ItemModel::After);});
    }
    if (!bkMenu->isEmpty()) {
        contextMenu.addMenu(bkMenu);
    }

    QMenu *pgMenu = new QMenu(tr("Add Page"));
    if (item->allowedChild(Item::Page)) {
        QAction *inAction = pgMenu->addAction(tr("Inside"));
        connect(inAction, &QAction::triggered, [this, index]{doAddChild(index, Item::Page, ItemModel::Inside);});
    }
    if (item->allowedSibling(Item::Page)) {
        QAction *bfAction = pgMenu->addAction(tr("Before"));
        connect(bfAction, &QAction::triggered, [this, index]{doAddChild(index, Item::Page, ItemModel::Before);});
        QAction *afAction = pgMenu->addAction(tr("After"));
        connect(afAction, &QAction::triggered, [this, index]{doAddChild(index, Item::Page, ItemModel::After);});
    }
    if (!pgMenu->isEmpty()) {
        contextMenu.addMenu(pgMenu);
    }

    QMenu *fdMenu = new QMenu(tr("Add Group"));
    if (item->allowedChild(Item::Group)) {
        QAction *inAction = fdMenu->addAction(root ? tr("Here") : tr("Inside"));
        connect(inAction, &QAction::triggered, [this, index]{doAddChild(index, Item::Group, ItemModel::Inside);});
    }
    if (item->allowedSibling(Item::Group)) {
        QAction *bfAction = fdMenu->addAction(tr("Before"));
        connect(bfAction, &QAction::triggered, [this, index]{doAddChild(index, Item::Group, ItemModel::Before);});
        QAction *afAction = fdMenu->addAction(tr("After"));
        connect(afAction, &QAction::triggered, [this, index]{doAddChild(index, Item::Group, ItemModel::After);});
    }
    if (!fdMenu->isEmpty()) {
        contextMenu.addMenu(fdMenu);
    }

    QMenu *ntMenu = new QMenu(tr("Add Note"));
    if (item->allowedChild(Item::Note)) {
        QAction *inAction = ntMenu->addAction(root ? tr("Here") : tr("Inside"));
        connect(inAction, &QAction::triggered, [this, index]{doAddChild(index, Item::Note, ItemModel::Inside);});
    }
    if (item->allowedSibling(Item::Note)) {
        QAction *bfAction = ntMenu->addAction(tr("Before"));
        connect(bfAction, &QAction::triggered, [this, index]{doAddChild(index, Item::Note, ItemModel::Before);});
        QAction *afAction = ntMenu->addAction(tr("After"));
        connect(afAction, &QAction::triggered, [this, index]{doAddChild(index, Item::Note, ItemModel::After);});
    }
    if (!ntMenu->isEmpty()) {
        contextMenu.addMenu(ntMenu);
    }

    contextMenu.exec(QWidget::mapToGlobal(pos));
}

/**!
 * @brief Slot triggered by the rename option in the context menu.
 *
 * @param bool unused.
 */
void GuiItemTree::doEditName(bool checked) {
    Q_UNUSED(checked);

    QModelIndex index = this->firstSelectedIndex();
    if (!index.isValid()) {
        qDebug() << "No item selected";
        return;
    }

    QString oldName = m_model->itemName(index);
    qDebug() << "Requested rename of item" << oldName;

    bool ok;
    QString newName = QInputDialog::getText(
        this, tr("Rename Story Item"), tr("New Name:"), QLineEdit::Normal, oldName, &ok
    );
    if (ok && !newName.isEmpty()) {
        m_model->setItemName(index, newName);
    }
}

/**!
 * @brief Slot triggered by the context menu on the tree.
 *
 * The slot will forward the call to create a new story item to the model.
 *
 * @param index the item index to add a child relative to.
 * @param type  the type of item to add.
 * @param loc   the relative location of where to add the new item.
 */
void GuiItemTree::doAddChild(const QModelIndex &index, Item::ItemType type, ItemModel::AddLocation loc) {
    if (m_model) {
        if (m_model->addItem(index, type, loc)) {
            qDebug() << "Added" << Item::typeToLabel(type);
        } else {
            qWarning() << "Failed to add" << Item::typeToLabel(type);
        }
    }
}

void GuiItemTree::saveExpanded(const QModelIndex &index) {
    if (m_model) m_model->setExpanded(index, true);
}

void GuiItemTree::saveCollapsed(const QModelIndex &index) {
    if (m_model) m_model->setExpanded(index, false);
}

} // namespace Collett
