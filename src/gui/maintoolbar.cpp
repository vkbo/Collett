/*
** Collett – GUI Main Tool Bar Class
** =================================
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

#include "maintoolbar.h"
#include "icons.h"

#include <QObject>
#include <QToolBar>
#include <QWidget>
#include <QSizePolicy>
#include <QAction>
#include <QMenu>

namespace Collett {

GuiMainToolBar::GuiMainToolBar(QWidget *parent)
    : QToolBar(parent)
{
    QWidget *stretch1 = new QWidget();
    QWidget *stretch2 = new QWidget();
    stretch1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    stretch2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_projectName = new QLabel(tr("No Project"));

    this->setOrientation(Qt::Horizontal);
    this->buildProjectMenu();
    this->addWidget(stretch1);
    this->addWidget(m_projectName);
    this->addWidget(stretch2);
    this->buildMoreMenu();
}

void GuiMainToolBar::setProjectName(const QString &name) {
    m_projectName->setText(name);
}

/**
 * Build Functions
 * ===============
 */

void GuiMainToolBar::buildProjectMenu() {

    CollettIcons *icons = CollettIcons::instance();

    // Menu
    m_projectMenu = new QMenu(this);

    // New Project
    m_newProject = m_projectMenu->addAction(tr("New Project"));

    // Open Project
    m_openProject = m_projectMenu->addAction(tr("Open Project"));

    // Save Project
    m_saveProject = m_projectMenu->addAction(tr("Save Project"));

    // Assemble
    m_projectButton = new QToolButton(this);
    m_projectButton->setText(tr("Project"));
    m_projectButton->setIcon(icons->icon("archive"));
    m_projectButton->setMenu(m_projectMenu);
    m_projectButton->setPopupMode(QToolButton::InstantPopup);
    this->addWidget(m_projectButton);

}

void GuiMainToolBar::buildMoreMenu() {

    CollettIcons *icons = CollettIcons::instance();

    // Menu
    m_moreMenu = new QMenu(this);

    // Assemble
    m_moreButton = new QToolButton(this);
    m_moreButton->setText(tr("Menu"));
    m_moreButton->setIcon(icons->icon("more"));
    m_moreButton->setMenu(m_moreMenu);
    m_moreButton->setPopupMode(QToolButton::InstantPopup);
    this->addWidget(m_moreButton);

}

} // namespace Collett
