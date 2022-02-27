/*
** Collett – Main GUI Class
** ========================
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
#include "guimain.h"
#include "settings.h"

#include <QMainWindow>
#include <QMessageBox>
#include <QApplication>
#include <QStandardPaths>

namespace Collett {

GuiMain::GuiMain(QWidget *parent) : QMainWindow(parent) {

    // Create Main Data Object
    m_data = CollettData::instance();
    m_data->newProject();

    // Collett Widgets
    m_treeToolBar = new GuiTreeToolBar(this);
    m_mainStatus  = new GuiMainStatus(this);
    m_workArea    = new GuiWorkArea(this);

    // Tree Stack
    m_treeStack = new QStackedWidget(this);

    // Assemble Main Window
    m_splitMain = new QSplitter(Qt::Horizontal, this);
    m_splitMain->setContentsMargins(0, 0, 0, 0);
    m_splitMain->setOpaqueResize(false);
    m_splitMain->addWidget(m_treeStack);
    m_splitMain->addWidget(m_workArea);

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

GuiMain::~GuiMain() {
    qDebug() << "Destructor: GuiMain";
    delete m_data;
}

/**
 * Project Methods
 * ===============
 */

void GuiMain::openProject(const QString &path) {
    m_data->openProject(path);
    if (!m_data->hasProject()) {
        return;
    }
    m_workArea->openDocument(m_data->project()->relativePath("Some Document.collett"));
}

void GuiMain::saveProject() {
    m_data->saveProject();
}

void GuiMain::closeProject() {
    m_data->closeProject();
};

/**
 * GUI Methods
 * ===========
 */

bool GuiMain::closeMain() {

    m_data->saveProject();
    m_data->closeProject();

    // Save Settings
    CollettSettings *mainConf = CollettSettings::instance();
    if (!this->isFullScreen()) {
        mainConf->setMainWindowSize(this->size());
        mainConf->setMainSplitSizes(m_splitMain->sizes());
    }
    mainConf->flushSettings();

    CollettIcons::destroy();
    CollettSettings::destroy();

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

/**
 * Private Slots
 * =============
 */

void GuiMain::closeProjectRequest() {
    auto response = QMessageBox::question(
        this, tr("Question"), tr("Do you want to close the project?")
    );
    if (response == QMessageBox::Yes) {
        this->closeProject();
    }
}

} // namespace Collett
