/*
** Collett – GUI Project Panel Class
** =================================
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

#ifndef COLLETT_GUI_PROJECT_PANEL_H
#define COLLETT_GUI_PROJECT_PANEL_H

#include "collett.h"
#include "projectview.h"

#include <QTreeView>
#include <QWidget>
#include <QVBoxLayout>

namespace Collett {

class GuiProjectPanel : public QWidget
{
    Q_OBJECT

public:
    explicit GuiProjectPanel(QWidget *parent = nullptr);
    ~GuiProjectPanel();

    // Methods
    void openProjectTasks();
    void closeProjectTasks();

    GuiProjectView *projectView = nullptr;

public slots:
    void createFile(const ItemLevel itemLevel) {if(projectView) projectView->createFile(itemLevel);};
    void createFolder() {if(projectView) projectView->createFolder();};
    void createRoot(const ItemClass itemClass) {if(projectView) projectView->createRoot(itemClass);};

};
} // namespace Collett

#endif // COLLETT_GUI_PROJECT_PANEL_H
