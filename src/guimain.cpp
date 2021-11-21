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
#include "maintoolbar.h"
#include "treetoolbar.h"
#include "statusbar.h"
#include "storytree.h"
#include "doceditor.h"

#include <QApplication>
#include <QCloseEvent>

namespace Collett {

GuiMain::GuiMain(QWidget *parent) : QMainWindow(parent) {

    // Collett Widgets
    m_mainToolBar = new GuiMainToolBar(this);
    m_treeToolBar = new GuiTreeToolBar(this);
    m_storyTree   = new GuiStoryTree(this);
    m_mainStatus  = new GuiMainStatus(this);
    m_docEditor   = new GuiDocEditor(this);

    // Assemble Main Window
    m_splitMain = new QSplitter(Qt::Horizontal, this);
    m_splitMain->setContentsMargins(4, 4, 4, 4);
    m_splitMain->setOpaqueResize(false);
    m_splitMain->addWidget(m_storyTree);
    m_splitMain->addWidget(m_docEditor);

    this->addToolBar(Qt::TopToolBarArea, m_mainToolBar);
    this->addToolBar(Qt::LeftToolBarArea, m_treeToolBar);
    this->setCentralWidget(m_splitMain);
    this->setStatusBar(this->m_mainStatus);

    // Apply Settings
    CollettSettings *mainConf = CollettSettings::instance();
    resize(mainConf->mainWindowSize());
    m_splitMain->setSizes(mainConf->mainSplitSizes());

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
