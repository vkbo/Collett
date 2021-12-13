/*
** Collett – Main GUI Class
** ========================
**
** This file is a part of Collett
** Copyright 2020–2021, Veronica Berglyd Olsen
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

#include "collett.h"
#include "data.h"
#include "maintoolbar.h"
#include "treetoolbar.h"
#include "statusbar.h"
#include "storytree.h"
#include "doceditor.h"

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QSplitter>
#include <QString>

namespace Collett {

class GuiMain : public QMainWindow
{
    Q_OBJECT

public:
    GuiMain(QWidget *parent=nullptr);
    ~GuiMain();

    void openProject(const QString &path);
    bool saveProject();
    bool closeProject();

    bool closeMain();

private:
    CollettData *m_data;

    // Collett Widgets
    GuiMainToolBar *m_mainToolBar;
    GuiTreeToolBar *m_treeToolBar;
    GuiStoryTree   *m_storyTree;
    GuiDocEditor   *m_docEditor;
    GuiMainStatus  *m_mainStatus;

    // GUI Widgets
    QSplitter *m_splitMain;

    // Events
    void closeEvent(QCloseEvent*);

};
} // namespace Collett

#endif // GUI_MAIN_H
