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

#ifndef GUI_MAIN_H
#define GUI_MAIN_H

#include "collett.h"
#include "data.h"
#include "projectpanel.h"
#include "projecttoolbar.h"
#include "settings.h"
#include "theme.h"
#include "workpanel.h"

#include <QMainWindow>
#include <QSplitter>
#include <QToolBar>

namespace Collett {

class GuiMain : public QMainWindow
{
    Q_OBJECT

public:
    GuiMain(QWidget *parent=nullptr);
    ~GuiMain();

    // Methods
    void openProject(const QString &path);
    void saveProject();
    void closeProject();
    bool closeMain();

private:
    // Singletons
    SharedData *m_data;
    Settings   *m_settings;
    Theme      *m_theme;

    // Components
    GuiProjectPanel *m_projectPanel;
    GuiWorkPanel    *m_workPanel;

    // ToolBars
    GuiProjectToolBar *m_projectToolBar;

    // Layout
    QSplitter *m_splitMain;

    // Events
    void closeEvent(QCloseEvent*);

private slots:

    void onProjectOpen();
    void onProjectSave() {saveProject();};
    void onProjectClose() {closeProject();};
    void updateTitle();

};
} // namespace Collett

#endif // GUI_MAIN_H
