/*
** Collett - GUI App ToolBar
** =========================
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

#include "apptoolbar.h"
#include "collett.h"
#include "mtoolbutton.h"

#include <QAction>
#include <QToolBar>
#include <QWidget>

namespace Collett {

// Constructor/Destructor
// ======================

GuiAppToolBar::GuiAppToolBar(QWidget *parent) : QToolBar(parent)
{
    actSettings = new QAction(tr("Settings"), this);

    MToolButton *btnSettings = new MToolButton(this);
    btnSettings->setDefaultAction(actSettings);
    btnSettings->setThemeIcon("settings", ThemeColor::ToolColor);
    this->addWidget(btnSettings);
}

GuiAppToolBar::~GuiAppToolBar()
{
    qDebug() << "Destructor: GuiAppToolBar";
}

} // namespace Collett
