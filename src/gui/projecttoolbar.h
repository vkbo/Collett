/*
** Collett – GUI Project ToolBar
** =============================
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

#ifndef COLLETT_GUI_PROJECT_TOOLBAR_H
#define COLLETT_GUI_PROJECT_TOOLBAR_H

#include "collett.h"
#include "theme.h"

#include <QAction>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>
#include <QWidget>

namespace Collett {

class GuiMain;
class GuiProjectToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit GuiProjectToolBar(GuiMain *parent);
    ~GuiProjectToolBar();

signals:
    void createFileRequested(ItemLevel itemLevel);
    void createFolderRequested();
    void createRootRequested(ItemClass itemClass);

private:
    Theme *m_theme;

    // Project
    QToolButton *btnProject;
    QMenu       *mnuProject;
    QAction     *actOpenProject;
    QAction     *actSaveProject;
    QAction     *actCloseProject;

    // Create New
    QToolButton *btnCreate;
    QMenu       *mnuCreate;
    QMenu       *mnuCreateRoot;

    // Helpers
    void addFileEntry(ItemLevel itemLevel);
    void addRootEntry(ItemClass itemClass);

    friend class GuiMain;
};
} // namespace Collett

#endif // COLLETT_GUI_PROJECT_TOOLBAR_H
