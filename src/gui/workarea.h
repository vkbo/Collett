/*
** Collett – GUI Work Area Class
** =============================
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

#ifndef GUI_WORKAREA_H
#define GUI_WORKAREA_H

#include <QObject>
#include <QTabWidget>

namespace Collett {

class GuiWorkArea : public QTabWidget
{
    Q_OBJECT

public:
    GuiWorkArea(QWidget *parent=nullptr);
    ~GuiWorkArea() {};

};
} // namespace Collett

#endif // GUI_WORKAREA_H
