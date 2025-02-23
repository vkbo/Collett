/*
** Collett – Main GUI Class
** ========================
**
** This file is a part of Collett
** Copyright 2025, Veronica Berglyd Olsen
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

#include "guimain.h"

#include <QApplication>
#include <QCloseEvent>
#include <QJsonArray>

namespace Collett {

GuiMain::GuiMain(QWidget *parent) : QMainWindow(parent) {

    return;
}

GuiMain::~GuiMain() {
    qDebug() << "Destructor: GuiMain";
}

/**
 * GUI Methods
 * ===========
 */

bool GuiMain::closeMain() {
    return true;
}

/**
 * Events
 * ======
 */

void GuiMain::closeEvent(QCloseEvent *event) {
    if (closeMain()) {
        event->accept();
    } else {
        event->ignore();
    }
}

} // namespace Collett
