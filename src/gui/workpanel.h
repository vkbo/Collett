/*
** Collett – GUI Work Panel Class
** ==============================
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

#ifndef COLLETT_GUI_WORK_PANEL_H
#define COLLETT_GUI_WORK_PANEL_H

#include "collett.h"

#include <QWidget>

namespace Collett {

class GuiWorkPanel : public QWidget
{
    Q_OBJECT

public:
    explicit GuiWorkPanel(QWidget *parent = nullptr);
    ~GuiWorkPanel();

};
} // namespace Collett

#endif // COLLETT_GUI_PROJECT_PANEL_H
