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

#ifndef GUI_MAIN_H
#define GUI_MAIN_H

#include "data.h"
#include "collett.h"
#include "workarea.h"
#include "statusbar.h"
#include "treetoolbar.h"

#include <QObject>
#include <QString>
#include <QWidget>
#include <QSplitter>
#include <QCloseEvent>
#include <QMainWindow>

namespace Collett {

class GuiMain : public QMainWindow
{
    Q_OBJECT

public:
    GuiMain(QWidget *parent=nullptr);
    ~GuiMain();

    // Collection Methods

    void openCollection(const QString &path);
    void saveCollection();
    void closeCollection();

    // GUI Methods

    bool closeMain();

private:
    CollettData *m_data;

    // Collett Widgets
    GuiTreeToolBar *m_treeToolBar;
    GuiWorkArea    *m_workArea;
    GuiMainStatus  *m_mainStatus;

    // Events
    void closeEvent(QCloseEvent*);

private slots:

    void closeCollectionRequest();

};
} // namespace Collett

#endif // GUI_MAIN_H
