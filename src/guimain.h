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

#ifndef GUI_MAIN_H
#define GUI_MAIN_H

#include "data.h"
#include "item.h"
#include "collett.h"
#include "itemtree.h"
#include "doceditor.h"
#include "statusbar.h"
#include "maintoolbar.h"
#include "treetoolbar.h"

#include <QHash>
#include <QUuid>
#include <QObject>
#include <QString>
#include <QWidget>
#include <QSplitter>
#include <QMainWindow>
#include <QModelIndex>
#include <QStackedWidget>

namespace Collett {

class GuiMain : public QMainWindow
{
    Q_OBJECT

public:
    GuiMain(QWidget *parent=nullptr);
    ~GuiMain();

    // Project Methods

    void openProject(const QString &path);
    bool saveProject();
    bool closeProject();

    // Document Methods

    void openDocument(Item *item);
    void saveDocument();
    void closeDocument();

    // GUI Methods

    void addItemTree(const QString &name);
    bool closeMain();

private:
    CollettData *m_data;

    // Collett Widgets
    GuiMainToolBar *m_mainToolBar;
    GuiTreeToolBar *m_treeToolBar;
    QStackedWidget *m_treeStack;
    GuiDocEditor   *m_docEditor;
    GuiMainStatus  *m_mainStatus;

    QHash<QString, GuiItemTree*> m_itemTrees;

    // GUI Widgets
    QSplitter *m_splitMain;

    // Events
    void closeEvent(QCloseEvent*);

private slots:

    void closeProjectRequest();
    void openSelectedDocument();
    void saveCurrentDocument();
    void renameDocument();
    void itemTreeDoubleClick(const QModelIndex &index);
    void changeModelTree(GuiItemTree *tree);

};
} // namespace Collett

#endif // GUI_MAIN_H
