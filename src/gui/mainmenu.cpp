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

#include "collett.h"
#include "mainmenu.h"
#include "projectview.h"

#include <QAction>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QWidget>

namespace Collett {

// Constructor/Destructor
// ======================

GuiMainMenu::GuiMainMenu(GuiProjectView *view, QWidget *parent) : QMenuBar(parent)
{
    this->buildProjectMenu(view);
}

GuiMainMenu::~GuiMainMenu()
{
    qDebug() << "Destructor: GuiMainMenu";
}

// Menu Builders
// =============

/**! @brief Build the Project menu.
 *
 * The edit and delete item actions are owned by the project view and are only
 * referenced here, so they stay in sync with the tree's context menu.
 */
void GuiMainMenu::buildProjectMenu(GuiProjectView *view)
{
    mnuProject = this->addMenu(tr("&Project"));

    actOpenProject = mnuProject->addAction(tr("Create or Open Project"));
    actOpenProject->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O));

    actSaveProject = mnuProject->addAction(tr("Save Project"));
    actSaveProject->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));

    actCloseProject = mnuProject->addAction(tr("Close Project"));
    actCloseProject->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_W));

    mnuProject->addSeparator();
    mnuProject->addAction(view->actEditItem);
    mnuProject->addAction(view->actDeleteItem);

    mnuProject->addSeparator();
    actExit = mnuProject->addAction(tr("Exit"));
    actExit->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
}

} // namespace Collett
