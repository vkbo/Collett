/*
** Collett – GUI Project Tree Class
** ================================
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

#include "collett.h"
#include "mtreeview.h"
#include "projectview.h"
#include "projectmodel.h"
#include "edititem.h"

#include <QAbstractItemView>
#include <QAction>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QTreeView>
#include <QUuid>
#include <QWidget>

namespace Collett {

// Constructor/Destructor
// ======================

GuiProjectView::GuiProjectView(QWidget *parent) : MTreeView(parent) {

    m_data = SharedData::instance();
    m_theme = Theme::instance();

    this->setIconSize(m_theme->baseIconSize());
    this->setUniformRowHeights(true);
    this->setAllColumnsShowFocus(true);
    this->setExpandsOnDoubleClick(true);
    this->setAutoExpandDelay(1000);
    this->setHeaderHidden(true);
    this->setIndentation(m_theme->baseIconHeight());

    // Allow Move by Drag & Drop
    this->setDragEnabled(true);
    this->setDragDropMode(QAbstractItemView::InternalMove);
    
    // Set selection options
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Create Actions
    actEditItem = new QAction(tr("Edit Project Item"), this);
    actEditItem->setShortcut(QKeySequence(Qt::Key_F2));
    actEditItem->setShortcutContext(Qt::WidgetShortcut);
    this->addAction(actEditItem);

    actDeleteItem = new QAction(tr("Delete Project Item"), this);
    actDeleteItem->setShortcut(QKeySequence(Qt::Key_Delete));
    actDeleteItem->setShortcutContext(Qt::WidgetShortcut);
    this->addAction(actDeleteItem);

    // Connect Signals
    this->connect(this, &GuiProjectView::expanded, this, &GuiProjectView::onNodeExpanded);
    this->connect(this, &GuiProjectView::collapsed, this, &GuiProjectView::onNodeCollapsed);
    this->connect(actEditItem, &QAction::triggered, this, &GuiProjectView::editSelectedItem);
    this->connect(actDeleteItem, &QAction::triggered, this, &GuiProjectView::deleteSelectedItem);
}

GuiProjectView::~GuiProjectView() {
    qDebug() << "Destructor: GuiProjectView";
}

// Public Methods
// ==============

void GuiProjectView::openProjectTasks() {
    ProjectModel *model = this->getModel();
    if (model) {
        QItemSelectionModel *m = this->selectionModel();
        this->setModel(model);
        delete m;
        this->adjustHeaders();
        this->restoreExpandedState();
    }
}

void GuiProjectView::closeProjectTasks() {
    QItemSelectionModel *m = this->selectionModel();
    this->setModel(nullptr);
    delete m;
}

// Private Getters
// ===============

ProjectModel *GuiProjectView::getModel() {
    if (m_data->hasProject()) {
        return m_data->project()->tree()->model();
    }
    return nullptr;
}

Node *GuiProjectView::getNode(const QModelIndex &index) {
    if (m_data->hasProject()) {
        ProjectModel *model = m_data->project()->tree()->model();
        if (model) return model->nodeAtIndex(index);
    }
    return nullptr;
}

// Private Methods
// ===============

void GuiProjectView::adjustHeaders() {
    QHeaderView *header = this->header();
    if (header) {
        header->setStretchLastSection(false);
        header->setSectionResizeMode(0, QHeaderView::Stretch);
        header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        header->setSectionResizeMode(2, QHeaderView::Fixed);
        header->setSectionResizeMode(3, QHeaderView::Fixed);
        header->resizeSection(2, m_theme->baseButtonHeight() + 6);
        header->resizeSection(3, m_theme->baseButtonHeight() + 6);
    }
}

void GuiProjectView::restoreExpandedState() {
    ProjectModel *model = this->getModel();
    if (model) {
        this->blockSignals(true);
        for (QModelIndex index : model->allExpanded()) {
            this->setExpanded(index, true);
        }
        this->blockSignals(false);
    }
}

// Public Slots
// ============

void GuiProjectView::createFile(const ItemLevel itemLevel) {
    ProjectModel *model = this->getModel();
    QModelIndex current = this->currentIndex();
    if (model && current.isValid()) {
        Node *node = model->addFile(tr("New File"), itemLevel, current);
        if (node) EditItemDialog::editNode(this, node);
    }
}

void GuiProjectView::createFolder() {
    ProjectModel *model = this->getModel();
    QModelIndex current = this->currentIndex();
    if (model && current.isValid()) {
        Node *node = model->addFolder(tr("New Folder"), current);
        if (node) EditItemDialog::editNode(this, node);
    }
}

void GuiProjectView::createRoot(const ItemClass itemClass) {
    ProjectModel *model = this->getModel();
    QModelIndex current = this->currentIndex();
    if (model && current.isValid()) {
        Node *node = model->addRoot(tr("New Root"), itemClass, current);
        if (node) EditItemDialog::editNode(this, node);
    }
}

// Private Slots
// =============

void GuiProjectView::onNodeExpanded(const QModelIndex &index) {
    Node *node = this->getNode(index);
    if (node) node->setExpanded(true);
}

void GuiProjectView::onNodeCollapsed(const QModelIndex &index) {
    Node *node = this->getNode(index);
    if (node) node->setExpanded(false);
}

void GuiProjectView::editSelectedItem() {
    Node *node = this->getNode(this->currentIndex());
    if (node){
        EditItemDialog::editNode(this, node);
    }
}

void GuiProjectView::deleteSelectedItem() {
    Node *node = this->getNode(this->currentIndex());
    if (node){
    }
}

} // namespace Collett
