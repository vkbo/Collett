/*
** Collett – GUI Main Tool Bar Class
** =================================
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

#ifndef GUI_MAINTOOLBAR_H
#define GUI_MAINTOOLBAR_H

#include <QObject>
#include <QToolBar>
#include <QLabel>
#include <QString>
#include <QAction>
#include <QMenu>
#include <QToolButton>

namespace Collett {

class GuiMain;
class GuiMainToolBar : public QToolBar
{
    Q_OBJECT

public:
    GuiMainToolBar(QWidget *parent=nullptr);
    ~GuiMainToolBar() {};

    void setProjectName(const QString &name);

private:
    QLabel *m_projectName;

    // Project Menu
    QToolButton *m_projectButton;
    QMenu       *m_projectMenu;
    QAction     *m_newProject;
    QAction     *m_openProject;
    QAction     *m_saveProject;

    // Documents Menu
    QToolButton *m_docsButton;
    QMenu       *m_docsMenu;
    QAction     *m_newDocument;
    QAction     *m_openDocument;
    QAction     *m_saveDocument;
    QAction     *m_renameDocument;

    // DropDown Menu
    QToolButton *m_moreButton;
    QMenu       *m_moreMenu;

    void buildMainMenu();
    void buildMoreMenu();

    friend class GuiMain;

};
} // namespace Collett

#endif // GUI_MAINTOOLBAR_H
