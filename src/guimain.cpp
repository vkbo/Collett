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
#include "item.h"
#include "icons.h"
#include "guimain.h"
#include "itemtree.h"
#include "settings.h"
#include "doceditor.h"
#include "statusbar.h"
#include "maintoolbar.h"
#include "treetoolbar.h"

#include <QUuid>
#include <QString>
#include <QWidget>
#include <QCloseEvent>
#include <QMainWindow>
#include <QMessageBox>
#include <QModelIndex>
#include <QApplication>
#include <QStackedWidget>
#include <QItemSelectionModel>

namespace Collett {

GuiMain::GuiMain(QWidget *parent) : QMainWindow(parent) {

    // Create Main Data Object
    m_data = CollettData::instance();

    // Collett Widgets
    m_mainToolBar = new GuiMainToolBar(this);
    m_treeToolBar = new GuiTreeToolBar(this);
    m_treeStack   = new QStackedWidget(this);
    m_mainStatus  = new GuiMainStatus(this);
    m_docEditor   = new GuiDocEditor(this);

    // Assemble Main Window
    m_splitMain = new QSplitter(Qt::Horizontal, this);
    m_splitMain->setContentsMargins(0, 0, 0, 0);
    m_splitMain->setOpaqueResize(false);
    m_splitMain->addWidget(m_treeStack);
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
    connect(m_mainToolBar->m_closeProject, SIGNAL(triggered()),
            this, SLOT(closeProjectRequest()));
    connect(m_mainToolBar->m_openDocument, SIGNAL(triggered()),
            this, SLOT(openSelectedDocument()));
    connect(m_mainToolBar->m_saveDocument, SIGNAL(triggered()),
            this, SLOT(saveCurrentDocument()));
    connect(m_mainToolBar->m_renameDocument, SIGNAL(triggered()),
            this, SLOT(renameDocument()));
    connect(m_treeToolBar, SIGNAL(treeButtonClicked(GuiItemTree*)),
            this, SLOT(changeModelTree(GuiItemTree*)));

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
    qDeleteAll(m_itemTrees.begin(), m_itemTrees.end());
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

    for (const QString &name : m_data->project()->modelList()) {
        qDebug() << "Adding tree:" << name;
        this->addItemTree(name);
    }
    if (m_itemTrees.contains("story")) {
        this->m_treeStack->setCurrentWidget(m_itemTrees.value("story"));
    }

    QUuid lastDocMain = m_data->project()->lastDocumentMain();
    if (!lastDocMain.isNull()) {
        Item *itemMain = nullptr;
        for (const GuiItemTree *tree : m_itemTrees) {
            itemMain = static_cast<ItemModel*>(tree->model())->itemFromHandle(lastDocMain);
            if (itemMain) break;
        }
        this->openDocument(itemMain);
    }

    m_mainToolBar->setProjectName(m_data->project()->projectName());
};

bool GuiMain::saveProject() {
    this->saveDocument();
    return true;
}

bool GuiMain::closeProject() {
    this->closeDocument();
    m_treeToolBar->clearModels();
    m_data->closeProject();
    return true;
};

/**
 * Document Methods
 * ================
 */

void GuiMain::openDocument(Item *item) {

    if (!m_data->hasProject() || !item) {
        return;
    }
    if (!item->canHoldDocument()) {
        return;
    }

    if (m_docEditor->hasDocument()) {
        m_docEditor->saveDocument();
        m_docEditor->closeDocument();
    }
    m_docEditor->openDocument(item);
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

void GuiMain::addItemTree(const QString &name) {

    if (!m_data->hasProject()) {
        return;
    }
    if (m_itemTrees.contains(name)) {
        qWarning() << "ItemTree already exists:" << name;
        return;
    }

    ItemModel *model = m_data->project()->model(name);
    if (!model) {
        qWarning() << "Model does not exist:" << name;
        return;
    }

    GuiItemTree *tree = new GuiItemTree(this);
    m_itemTrees.insert(name, tree);

    tree->setTreeModel(model);
    m_treeStack->addWidget(tree);
    m_treeToolBar->addModelEntry(name, model, tree);

    connect(tree, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(itemTreeDoubleClick(const QModelIndex&)));
}

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

void GuiMain::closeProjectRequest() {
    auto response = QMessageBox::question(
        this, tr("Question"), tr("Do you want to close the project?")
    );
    if (response == QMessageBox::Yes) {
        this->closeProject();
    }
}

void GuiMain::openSelectedDocument() {
    if (!m_data->hasProject())
        return;

    GuiItemTree *tree = static_cast<GuiItemTree*>(m_treeStack->currentWidget());
    QModelIndex index = tree->firstSelectedIndex();
    if (!index.isValid())
        return;

    ItemModel *model = static_cast<ItemModel*>(tree->model());
    this->openDocument(model->itemFromIndex(index));
}

void GuiMain::saveCurrentDocument() {
    if (!m_data->hasProject())
        return;
    if (m_docEditor->anyFocus())
        m_docEditor->saveDocument();
}

void GuiMain::renameDocument() {
    if (!m_data->hasProject())
        return;

    GuiItemTree *tree = static_cast<GuiItemTree*>(m_treeStack->currentWidget());
    tree->doEditName(false);
}

void GuiMain::itemTreeDoubleClick(const QModelIndex &index) {
    Q_UNUSED(index);
    this->openSelectedDocument();
}

void GuiMain::changeModelTree(GuiItemTree *tree) {
    if (!m_data->hasProject())
        return;

    m_treeStack->setCurrentWidget(static_cast<QWidget*>(tree));
}

} // namespace Collett
