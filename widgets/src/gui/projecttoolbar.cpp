/*
** Collett - GUI Project ToolBar
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

#include "constants.h"
#include "mtoolbutton.h"
#include "projecttoolbar.h"
#include "theme.h"

#include <QAction>
#include <QMenu>
#include <QSize>
#include <QToolBar>
#include <QToolButton>
#include <QWidget>

namespace Collett {

// Constructor/Destructor
// ======================

GuiProjectToolBar::GuiProjectToolBar(QWidget *parent) : QToolBar(parent)
{
    m_theme = Theme::instance();

    // Create Button
    btnCreate = new MToolButton(this);
    mnuCreate = new QMenu(btnCreate);
    this->addFileEntry(ItemLevel::SceneLevel);
    this->addFileEntry(ItemLevel::ChapterLevel);
    this->addFileEntry(ItemLevel::TitleLevel);
    this->addFileEntry(ItemLevel::PageLevel);
    this->addFileEntry(ItemLevel::NoteLevel);

    QAction *actCreateFolder = mnuCreate->addAction(tr("Folder"));
    actCreateFolder->setIcon(m_theme->icons()->getProjectIcon(
        ItemType::FolderType, ItemClass::NovelClass, ItemLevel::PageLevel, m_theme->baseIconSize()
    ));
    connect(actCreateFolder, &QAction::triggered, this, &GuiProjectToolBar::createFolderRequested);

    mnuCreateRoot = mnuCreate->addMenu(tr("Root Folder"));
    this->addRootEntry(ItemClass::NovelClass);
    this->addRootEntry(ItemClass::CharacterClass);
    this->addRootEntry(ItemClass::PlotClass);
    this->addRootEntry(ItemClass::LocationClass);
    this->addRootEntry(ItemClass::ObjectClass);
    this->addRootEntry(ItemClass::EntityClass);
    this->addRootEntry(ItemClass::CustomClass);
    mnuCreateRoot->addSeparator();
    this->addRootEntry(ItemClass::ArchiveClass);

    btnCreate->setThemeIcon("add", ThemeColor::AddColor);
    btnCreate->setMenu(mnuCreate);
    btnCreate->setPopupMode(QToolButton::InstantPopup);
    this->addWidget(btnCreate);
}

GuiProjectToolBar::~GuiProjectToolBar()
{
    qDebug() << "Destructor: GuiProjectToolBar";
}

// Private Helpers
// ===============

void GuiProjectToolBar::addFileEntry(ItemLevel itemLevel)
{
    QAction *action = mnuCreate->addAction(itemLevelNames(itemLevel));
    action->setIcon(m_theme->icons()->getProjectIcon(
        ItemType::FileType, ItemClass::NovelClass, itemLevel, m_theme->baseIconSize()
    ));
    connect(action, &QAction::triggered, this, [=]() { emit createFileRequested(itemLevel); });
}

void GuiProjectToolBar::addRootEntry(ItemClass itemClass)
{
    QAction *action = mnuCreateRoot->addAction(itemClassNames(itemClass));
    action->setIcon(m_theme->icons()->getProjectIcon(
        ItemType::RootType, itemClass, ItemLevel::PageLevel, m_theme->baseIconSize()
    ));
    connect(action, &QAction::triggered, this, [=]() { emit createRootRequested(itemClass); });
}

} // namespace Collett
