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

#include "guimain.h"
#include "settings.h"
#include "mainmenu.h"
#include "maintoolbar.h"
#include "statusbar.h"
#include "storytree.h"
#include "doceditor.h"

#include <QApplication>
#include <QCloseEvent>

namespace Collett {

GuiMain::GuiMain(QWidget *parent) : QMainWindow(parent) {

    // Collett Widgets
    m_mainMenu    = new GuiMainMenu(this);
    m_mainToolBar = new GuiMainToolBar(this);
    m_storyTree   = new GuiStoryTree(this);
    m_mainStatus  = new GuiMainStatus(this);
    m_editOne     = new GuiDocEditor(this);
    m_editTwo     = new GuiDocEditor(this);

    // Assemble Main Window
    this->setMenuBar(this->m_mainMenu);
    this->setStatusBar(this->m_mainStatus);

    m_splitEdit = new QSplitter(Qt::Horizontal, this);
    m_splitEdit->addWidget(m_editOne);
    m_splitEdit->addWidget(m_editTwo);

    m_splitMain = new QSplitter(Qt::Horizontal, this);
    m_splitMain->setContentsMargins(4, 4, 4, 4);
    m_splitMain->setOpaqueResize(false);
    m_splitMain->addWidget(m_storyTree);
    m_splitMain->addWidget(m_splitEdit);

    this->setCentralWidget(m_splitMain);

    // Apply Settings
    CollettSettings *mainConf = CollettSettings::instance();
    resize(mainConf->mainWindowSize());

    // Finalise
    setWindowTitle(
        tr("%1 %2 Version %3").arg(qApp->applicationName(), "–", qApp->applicationVersion())
    );

    return;
}

/*
    GUI Functions
    =============
*/

bool GuiMain::closeMain() {

    // Save Settings
    CollettSettings *mainConf = CollettSettings::instance();
    if (!this->isFullScreen()) {
        mainConf->setMainWindowSize(this->size());
        mainConf->setMainSplitSizes(m_splitMain->sizes());
    }
    mainConf->flushSettings();

    return true;
}

/*
    Events
    ======
*/

void GuiMain::closeEvent(QCloseEvent *event) {
    if (closeMain()) {
        event->accept();
    } else {
        event->ignore();
    }
}

} // namespace Collett