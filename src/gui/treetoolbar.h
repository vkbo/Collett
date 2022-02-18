/*
** Collett – GUI Tree ToolBar Class
** ================================
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

#ifndef GUI_TREETOOLBAR_H
#define GUI_TREETOOLBAR_H

#include "itemtree.h"
#include "itemmodel.h"

#include <QHash>
#include <QAction>
#include <QObject>
#include <QString>
#include <QToolBar>
#include <QToolButton>

namespace Collett {

class GuiTreeToolBar : public QToolBar
{
    Q_OBJECT

public:
    GuiTreeToolBar(QWidget *parent=nullptr);
    ~GuiTreeToolBar() {};

    // Class Methods

    void clearModels();
    void addModelEntry(const QString &name, ItemModel *model, GuiItemTree *tree);

signals:
    void treeButtonClicked(GuiItemTree *tree);

private:
    QAction *m_addAction;
    QHash<QString, QToolButton*>  m_modelButtons;
    QHash<QString, GuiItemTree*> m_modelTree;

    void initToolBar();

private slots:
    void treeButtonTriggered(const QString &name);

};
} // namespace Collett

#endif // GUI_TREETOOLBAR_H
