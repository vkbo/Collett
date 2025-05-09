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

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QMenu>
#include <QSplitter>
#include <QToolButton>

using namespace Qt::Literals::StringLiterals;

namespace Collett {

// Constructor/Destructor
// ======================

GuiMain::GuiMain(QWidget *parent) : QMainWindow(parent) {

    // Static Objects
    m_data = SharedData::instance();
    m_settings = Settings::instance();
    m_theme = Theme::instance();

    // Panels
    projectPanel = new GuiProjectPanel(this);
    workPanel    = new GuiWorkPanel(this);

    // Main Splitter
    QList<int> sizes = {400, 1000};
    m_splitMain = new QSplitter(Qt::Horizontal, this);
    m_splitMain->setContentsMargins(0, 0, 0, 0);
    m_splitMain->setOpaqueResize(false);
    m_splitMain->addWidget(projectPanel);
    m_splitMain->addWidget(workPanel);
    m_splitMain->setSizes(m_settings->mainSplitSizes());

    // ToolBars
    projectToolBar = new GuiProjectToolBar(this);

    // Connect Signals
    connect(m_data, &SharedData::projectLoaded, this, &GuiMain::updateTitle);

    connect(projectToolBar->actOpenProject, &QAction::triggered, this, &GuiMain::onProjectOpen);
    connect(projectToolBar->actSaveProject, &QAction::triggered, this, &GuiMain::onProjectSave);
    connect(projectToolBar->actCloseProject, &QAction::triggered, this, &GuiMain::onProjectClose);

    connect(projectToolBar, &GuiProjectToolBar::createFileRequested, projectPanel, &GuiProjectPanel::createFile);
    connect(projectToolBar, &GuiProjectToolBar::createFolderRequested, projectPanel, &GuiProjectPanel::createFolder);
    connect(projectToolBar, &GuiProjectToolBar::createRootRequested, projectPanel, &GuiProjectPanel::createRoot);

    // Assemble
    this->setCentralWidget(m_splitMain);
    this->addToolBar(projectToolBar);

    // Apply Settings
    this->resize(m_settings->mainWindowSize());
    this->updateTitle();
}

GuiMain::~GuiMain() {
    qDebug() << "Destructor: GuiMain";
}

// Public Methods
// ==============

void GuiMain::openProject(const QString &path) {

    m_data->openProject(path);
    if (!m_data->hasProject()) {
        return;
    }
    projectPanel->openProjectTasks();
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
    projectPanel->closeProjectTasks();
}

bool GuiMain::closeMain() {
    this->saveProject();
    this->closeProject();

    // Save Settings
    if (!this->isFullScreen()) {
        m_settings->setMainWindowSize(this->size());
        m_settings->setMainSplitSizes(m_splitMain->sizes());
    }
    m_settings->flushSettings();

    m_theme->destroy();
    m_settings->destroy();

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

// Private Slots
// =============

void GuiMain::onProjectOpen() {

}

void GuiMain::updateTitle() {
    if (m_data->hasProject()) {
        setWindowTitle(QString("%1 - %2").arg(m_data->project()->data()->name(), qApp->applicationName()));
    } else {
        setWindowTitle(qApp->applicationName());
    }
}

} // namespace Collett
