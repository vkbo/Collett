/*
Collett – GUI Tree Tool Bar Class
=================================

This file is a part of Collett
Copyright 2020–2021, Veronica Berglyd Olsen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "treetoolbar.h"

#include <QObject>
#include <QToolBar>
#include <QWidget>
#include <QSizePolicy>

namespace Collett {

GuiTreeToolBar::GuiTreeToolBar(QWidget *parent)
    : QToolBar(parent)
{
    QWidget *stretch = new QWidget(this);
    stretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    this->setOrientation(Qt::Vertical);
    this->addAction(tr("Story"));
    this->addWidget(stretch);
    this->addAction(tr("Settings"));
}

} // namespace Collett
