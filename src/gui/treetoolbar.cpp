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

#include "treetoolbar.h"
#include "itemtree.h"
#include "itemmodel.h"
#include "icons.h"

#include <QHash>
#include <QAction>
#include <QObject>
#include <QString>
#include <QWidget>
#include <QToolBar>
#include <QSizePolicy>
#include <QToolButton>

namespace Collett {

GuiTreeToolBar::GuiTreeToolBar(QWidget *parent) : QToolBar(parent) {
    this->initToolBar();
}

/**
 * Class Methods
 * =============
 */

void GuiTreeToolBar::clearModels() {
    m_modelButtons.clear();
    m_modelTree.clear();
    this->clear();
    this->initToolBar();
}

void GuiTreeToolBar::addModelEntry(const QString &name, ItemModel *model, GuiItemTree *tree) {

    CollettIcons *icons = CollettIcons::instance();

    QToolButton *button = new QToolButton(this);
    button->setIcon(icons->icon(model->modelIcon()));
    button->setToolTip(model->modelName());
    this->insertWidget(m_addAction, button);

    m_modelButtons.insert(name, button);
    m_modelTree.insert(name, tree);

    connect(button, &QToolButton::clicked, [this, name]{treeButtonTriggered(name);});
}

/**
 * Internal Functions
 * ==================
 */

void GuiTreeToolBar::initToolBar() {
    QWidget *stretch = new QWidget(this);
    stretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    CollettIcons *icons = CollettIcons::instance();

    m_addAction = new QAction(this);
    m_addAction->setText(tr("Add Action"));
    m_addAction->setIcon(icons->icon("add"));

    this->setOrientation(Qt::Vertical);
    this->addAction(m_addAction);
    this->addWidget(stretch);
    this->addAction(icons->icon("settings"), tr("Settings"));
}

/**
 * Private Slots
 * =============
 */

void GuiTreeToolBar::treeButtonTriggered(const QString &name) {
    qDebug() << "Clicked tree button:" << name;
    if (m_modelTree.contains(name)) {
        emit treeButtonClicked(m_modelTree.value(name));
    } else {
    }
}

} // namespace Collett
