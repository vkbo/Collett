/*
** Collett - GUI Main Menu
** =======================
**
** This file is a part of Collett
** Copyright (C) 2026 Veronica Berglyd Olsen
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
#include "projectview.h"

#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QWidget>

namespace Collett {

class GuiMain;
class GuiMainMenu : public QMenuBar
{
    Q_OBJECT

public:
    explicit GuiMainMenu(GuiProjectView *view, QWidget *parent = nullptr);
    ~GuiMainMenu();

private:
    // Project Menu
    QMenu *mnuProject;
    QAction *actOpenProject;
    QAction *actSaveProject;
    QAction *actCloseProject;
    QAction *actExit;

    // Tools Menu
    QMenu *mnuTools;
    QAction *actPreferences;

    // Builders
    void buildProjectMenu(GuiProjectView *view);
    void buildToolsMenu();

    friend class GuiMain;
};
} // namespace Collett
