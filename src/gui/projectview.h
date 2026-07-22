/*
** Collett - GUI Project Tree Class
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

#pragma once

#include "collett.h"
#include "data.h"
#include "theme.h"
#include "mtreeview.h"

#include <QAction>
#include <QItemSelection>
#include <QModelIndex>
#include <QTreeView>
#include <QWidget>

namespace Collett {

class GuiProjectView : public MTreeView
{
    Q_OBJECT

public:
    explicit GuiProjectView(QWidget *parent = nullptr);
    ~GuiProjectView();

    // Methods
    void openProjectTasks();
    void closeProjectTasks();

    // Actions
    QAction *actEditItem;
    QAction *actDeleteItem;

signals:
    void nodeActivated(Node *node);

private:
    // Singletons
    SharedData *m_data;
    Theme *m_theme;

    // Getters
    ProjectModel *getModel();
    Node *getNode(const QModelIndex &index);

    // Methods
    void adjustHeaders();
    void restoreExpandedState();

public slots:
    void createFile(const ItemLevel itemLevel);
    void createFolder();
    void createRoot(const ItemClass itemClass);

private slots:
    void onNodeExpanded(const QModelIndex &index);
    void onNodeCollapsed(const QModelIndex &index);
    void onCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
    void editSelectedItem();
    void deleteSelectedItem();
};
} // namespace Collett
