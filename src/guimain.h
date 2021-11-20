/*
Collett – Main GUI Class
========================

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

#ifndef GUIMAIN_H
#define GUIMAIN_H

#include "collett.h"
#include "mainmenu.h"
#include "maintoolbar.h"
#include "statusbar.h"
#include "storytree.h"
#include "doceditor.h"

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QSplitter>

namespace Collett {

class GuiMain : public QMainWindow
{
    Q_OBJECT

public:
    GuiMain(QWidget *parent=nullptr);
    ~GuiMain() {};

    bool closeMain();

private:

    // Collett Widgets
    GuiMainMenu    *m_mainMenu;
    GuiMainToolBar *m_mainToolBar;
    GuiStoryTree   *m_storyTree;
    GuiMainStatus  *m_mainStatus;

    GuiDocEditor *m_editOne;
    GuiDocEditor *m_editTwo;

    // GUI Widgets
    QSplitter *m_splitMain;
    QSplitter *m_splitEdit;

    // Events
    void closeEvent(QCloseEvent*);

};
} // namespace Collett

#endif // GUIMAIN_H
