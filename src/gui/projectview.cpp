/*
** Collett – GUI Project View Class
** ================================
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

#include "icons.h"
#include "projectview.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace Collett {

GuiProjectView::GuiProjectView(QWidget *parent) : QWidget(parent) {

    CollettIcons *icons = CollettIcons::instance();
    QLabel *headerLabel = new QLabel(tr("Project"));

    m_listView = new QListView(this);

    m_addButton = new QToolButton();
    m_addButton->setIcon(icons->icon("add"));

    // Assemble
    QHBoxLayout *topBox = new QHBoxLayout();
    topBox->addWidget(headerLabel);
    topBox->addWidget(m_addButton);
    topBox->setContentsMargins(4, 6, 0, 0);

    QVBoxLayout *outerBox = new QVBoxLayout();
    outerBox->addLayout(topBox);
    outerBox->addWidget(m_listView);
    outerBox->setContentsMargins(0, 0, 0, 0);

    this->setLayout(outerBox);
}

} // namespace Collett
