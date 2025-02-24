/*
** Collett – Main GUI Class
** ========================
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

#include "guimain.h"

#include <QApplication>
#include <QCloseEvent>
#include <QSplitter>

namespace Collett {

// Constructor/Destructor
// ======================

GuiMain::GuiMain(QWidget *parent) : QMainWindow(parent) {

    m_data = CollettData::instance();
    setWindowTitle(qApp->applicationName());

    // Panels
    m_projectPanel = new GuiProjectPanel(this);
    m_workPanel    = new GuiWorkPanel(this);

    // Main Splitter
    QList<int> sizes = {400, 1000};
    m_splitMain = new QSplitter(Qt::Horizontal, this);
    m_splitMain->setContentsMargins(0, 0, 0, 0);
    m_splitMain->setOpaqueResize(false);
    m_splitMain->addWidget(m_projectPanel);
    m_splitMain->addWidget(m_workPanel);
    m_splitMain->setSizes(sizes);

    // Assemble
    this->setCentralWidget(m_splitMain);
    this->resize(1400, 900);
}

GuiMain::~GuiMain() {
    qDebug() << "Destructor: GuiMain";
}

// Public Methods

 /**!
 * @brief Open a Collett project.
 * 
 * @param path Path to the project.
 */
void GuiMain::openProject(const QString &path) {

    m_data->openProject(path);
    if (!m_data->hasProject()) {
        return;
    }
    m_projectPanel->openProjectTasks();
    setWindowTitle(m_data->project()->data()->name() + " - " + qApp->applicationName());
}

void GuiMain::saveProject() {
    if (m_data->hasProject()) {
        m_data->saveProject();
    }
}

void GuiMain::closeProject() {
    if (m_data->hasProject()) {
        m_data->closeProject();
    }
    m_projectPanel->closeProjectTasks();
}

bool GuiMain::closeMain() {
    this->saveProject();
    this->closeProject();
    return true;
}

// Events
// ======

void GuiMain::closeEvent(QCloseEvent *event) {
    if (closeMain()) {
        event->accept();
    } else {
        event->ignore();
    }
}

} // namespace Collett
