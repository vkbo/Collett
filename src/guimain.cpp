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

#include <QList>
#include <QMainWindow>
#include <QMessageBox>
#include <QApplication>

namespace Collett {

GuiMain::GuiMain(QWidget *parent) : QMainWindow(parent) {

    // Create Main Data Object
    m_data = CollettData::instance();
    m_data->newCollection();

    // Collett Widgets
    m_treeToolBar = new GuiTreeToolBar(this);
    m_mainStatus  = new GuiMainStatus(this);
    m_workArea    = new GuiWorkArea(this);

    // Assemble Main Window
    this->addToolBar(Qt::LeftToolBarArea, m_treeToolBar);
    this->setCentralWidget(m_workArea);
    this->setStatusBar(this->m_mainStatus);

    // Apply Settings
    CollettSettings *mainConf = CollettSettings::instance();
    resize(mainConf->mainWindowSize());

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
 * Collection Methods
 * ==================
 */

void GuiMain::openCollection(const QString &path) {
    m_data->openCollection(path);
    if (!m_data->hasCollection()) {
        return;
    }
    m_workArea->openDocument(m_data->collection()->relativePath("Some Document.collett"));
}

void GuiMain::saveCollection() {
    m_data->saveCollection();
}

void GuiMain::closeCollection() {
    m_data->closeCollection();
}

/**
 * GUI Methods
 * ===========
 */

bool GuiMain::closeMain() {

    m_data->saveCollection();
    m_data->closeCollection();

    // Save Settings
    CollettSettings *mainConf = CollettSettings::instance();
    if (!this->isFullScreen()) {
        mainConf->setMainWindowSize(this->size());
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

void GuiMain::closeCollectionRequest() {
    auto response = QMessageBox::question(
        this, tr("Question"), tr("Do you want to close the collection?")
    );
    if (response == QMessageBox::Yes) {
        this->closeCollection();
    }
}

} // namespace Collett
