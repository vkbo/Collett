/*
** Collett - GUI App ToolBar
** =========================
**
** This file is a part of Collett
** Copyright (C) 2026 Veronica Berglyd Olsen
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

#include "apptoolbar.h"
#include "collett.h"
#include "mtoolbutton.h"
#include "settings.h"
#include "theme.h"

#include <QAction>
#include <QToolBar>
#include <QWidget>

namespace Collett {

// Constructor/Destructor
// ======================

GuiAppToolBar::GuiAppToolBar(QWidget *parent) : QToolBar(parent)
{
    m_settings = Settings::instance();
    m_theme = Theme::instance();

    // Theme Mode
    m_btnTheme = new MToolButton(this);
    this->addWidget(m_btnTheme);
    this->updateThemeMode();
    connect(m_btnTheme, &MToolButton::clicked, this, &GuiAppToolBar::cycleThemeMode);

    // Settings
    actSettings = new QAction(tr("Settings"), this);

    MToolButton *btnSettings = new MToolButton(this);
    btnSettings->setDefaultAction(actSettings);
    btnSettings->setThemeIcon("settings", ThemeColor::ToolColor);
    this->addWidget(btnSettings);
}

GuiAppToolBar::~GuiAppToolBar()
{
    qDebug() << "Destructor: GuiAppToolBar";
}

// Public Slots
// ============

/**! @brief Set the theme button icon and tool tip to match the theme mode.
 */
void GuiAppToolBar::updateThemeMode()
{
    switch (m_settings->themeMode()) {
    case ThemeMode::LightTheme:
        m_btnTheme->setThemeIcon("theme_light", ThemeColor::ToolColor);
        m_btnTheme->setToolTip(tr("Light Theme"));
        break;
    case ThemeMode::DarkTheme:
        m_btnTheme->setThemeIcon("theme_dark", ThemeColor::ToolColor);
        m_btnTheme->setToolTip(tr("Dark Theme"));
        break;
    default:
        m_btnTheme->setThemeIcon("theme_auto", ThemeColor::ToolColor);
        m_btnTheme->setToolTip(tr("System Theme"));
        break;
    }
}

// Private Slots
// =============

/**! @brief Step the theme mode from system to light to dark and back.
 */
void GuiAppToolBar::cycleThemeMode()
{
    switch (m_settings->themeMode()) {
    case ThemeMode::AutoTheme: m_settings->setThemeMode(ThemeMode::LightTheme); break;
    case ThemeMode::LightTheme: m_settings->setThemeMode(ThemeMode::DarkTheme); break;
    default: m_settings->setThemeMode(ThemeMode::AutoTheme); break;
    }
    m_theme->loadTheme();
    this->updateThemeMode();
}

} // namespace Collett
