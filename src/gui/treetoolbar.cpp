/*
** Collett – GUI Tree ToolBar Class
** ================================
**
** This file is a part of Collett
** Copyright 2021–2022, Veronica Berglyd Olsen
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

#include "icons.h"
#include "treetoolbar.h"

#include <QFont>
#include <QSize>
#include <QSizePolicy>

namespace Collett {

GuiTreeToolBar::GuiTreeToolBar(QWidget *parent) : QToolBar(parent) {

    // ToolBar Settings
    this->setMovable(false);
    this->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    this->setIconSize(QSize(32, 32));

    // Under-Icon Text Font Size
    QFont font = this->font();
    font.setPointSizeF(0.7*font.pointSizeF());
    this->setFont(font);

    // ToolBar Action Buttons
    CollettIcons *icons = CollettIcons::instance();

    m_collectionAction = new QAction(this);
    m_collectionAction->setIcon(icons->icon("sideCollection"));
    m_collectionAction->setText(tr("Collection"));

    m_exploreAction = new QAction(this);
    m_exploreAction->setIcon(icons->icon("sideExplore"));
    m_exploreAction->setText(tr("Explore"));

    m_settingsAction = new QAction(this);
    m_settingsAction->setIcon(icons->icon("sideSettings"));
    m_settingsAction->setText(tr("Settings"));

    // Stretchable Widget
    QWidget *stretch = new QWidget(this);
    stretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Assemble
    this->setOrientation(Qt::Vertical);
    this->addAction(m_collectionAction);
    this->addAction(m_exploreAction);
    this->addWidget(stretch);
    this->addAction(m_settingsAction);
}

} // namespace Collett
