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
#include "settings.h"
#include "workpanel.h"

#include <QMainWindow>
#include <QSplitter>

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
    CollettData *m_data;
    Settings    *m_settings;

    // Components
    GuiProjectPanel *m_projectPanel;
    GuiWorkPanel    *m_workPanel;

    // Layout
    QSplitter *m_splitMain;

    void closeEvent(QCloseEvent*);

private slots:

};
} // namespace Collett

#endif // GUI_MAIN_H
