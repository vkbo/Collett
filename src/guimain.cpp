/*
** Collett – Main GUI Class
** ========================
**
** This file is a part of Collett
** Copyright 2020–2022, Veronica Berglyd Olsen
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

#include "data.h"
#include "doceditor.h"
#include "guimain.h"
#include "icons.h"
#include "maintoolbar.h"
#include "settings.h"
#include "statusbar.h"
#include "storytree.h"
#include "treetoolbar.h"

#include <QUuid>
#include <QString>
#include <QWidget>
#include <QCloseEvent>
#include <QMainWindow>
#include <QModelIndex>
#include <QApplication>
#include <QItemSelectionModel>

namespace Collett {

GuiMain::GuiMain(QWidget *parent) : QMainWindow(parent) {

    // Create Main Data Object
    m_data = CollettData::instance();

    // Collett Widgets
    m_mainToolBar = new GuiMainToolBar(this);
    m_treeToolBar = new GuiTreeToolBar(this);
    m_storyTree   = new GuiStoryTree(this);
    m_mainStatus  = new GuiMainStatus(this);
    m_docEditor   = new GuiDocEditor(this);

    // Assemble Main Window
    m_splitMain = new QSplitter(Qt::Horizontal, this);
    m_splitMain->setContentsMargins(0, 0, 0, 0);
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

    // Connect Signals and Slots
    connect(m_storyTree, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(storyTreeDoubleClick(const QModelIndex&)));

    connect(m_mainToolBar->m_openDocument, SIGNAL(triggered()),
            this, SLOT(openSelectedDocument()));
    connect(m_mainToolBar->m_saveDocument, SIGNAL(triggered()),
            this, SLOT(saveOpenDocument()));
    connect(m_mainToolBar->m_renameDocument, SIGNAL(triggered()),
            this, SLOT(renameDocument()));

    // Connect Actions to Capture Key Sequence
    this->addAction(m_mainToolBar->m_newProject);
    this->addAction(m_mainToolBar->m_openProject);
    this->addAction(m_mainToolBar->m_saveProject);
    this->addAction(m_mainToolBar->m_newDocument);
    this->addAction(m_mainToolBar->m_openDocument);
    this->addAction(m_mainToolBar->m_saveDocument);
    this->addAction(m_mainToolBar->m_renameDocument);

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

    QItemSelectionModel *m = m_storyTree->selectionModel();
    m_storyTree->setTreeModel(m_data->storyModel());
    delete m;

    QUuid lastDocMain = m_data->project()->lastDocumentMain();
    if (!lastDocMain.isNull()) {
        this->openDocument(lastDocMain);
    }

    m_mainToolBar->setProjectName(m_data->project()->projectName());
};

bool GuiMain::saveProject() {
    this->saveDocument();
    return true;
}

bool GuiMain::closeProject() {
    return true;
};

/**
 * Document Methods
 * ================
 */

void GuiMain::openDocument(const QUuid &uuid) {

    if (!m_data->hasProject()) {
        return;
    }

    if (m_docEditor->hasDocument()) {
        m_docEditor->saveDocument();
        m_docEditor->closeDocument();
    }
    m_docEditor->openDocument(uuid);
    m_data->project()->setLastDocumentMain(m_docEditor->currentDocument());
}

void GuiMain::saveDocument() {

    if (!m_data->hasProject()) {
        return;
    }
    if (m_docEditor->hasDocument()) {
        m_docEditor->saveDocument();
    }
}

void GuiMain::closeDocument() {
    m_docEditor->closeDocument();
}

/**
 * GUI Methods
 * ===========
 */

bool GuiMain::closeMain() {

    m_docEditor->saveDocument();
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

void GuiMain::openSelectedDocument() {
    if (!m_data->hasProject())
        return;

    QModelIndex index = m_storyTree->firstSelectedIndex();
    if (!index.isValid())
        return;

    this->openDocument(m_data->storyModel()->itemHandle(index));
}

void GuiMain::saveOpenDocument() {
    if (!m_data->hasProject())
        return;
    if (m_docEditor->anyFocus())
        m_docEditor->saveDocument();
}

void GuiMain::renameDocument() {
    if (!m_data->hasProject())
        return;
    m_storyTree->doEditName(false);
}

void GuiMain::storyTreeDoubleClick(const QModelIndex &index) {
    if (!m_data->hasProject() || !index.isValid()) {
        return;
    }
    this->openDocument(m_data->storyModel()->itemHandle(index));
}

} // namespace Collett
