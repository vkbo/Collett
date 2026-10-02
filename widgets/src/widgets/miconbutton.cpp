/*
** Collett - Icon Button Widget
** ============================
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

#include "collett.h"
#include "miconbutton.h"
#include "theme.h"

#include <QSize>
#include <QString>
#include <QToolButton>
#include <QWidget>

namespace Collett {

namespace {
// Button side length relative to the font height
constexpr qreal buttonScale = 1.5;
} // namespace

// Constructor/Destructor
// ======================

/**! @brief An icon-only tool button that looks like a square push button.
 *
 * It is meant for placing next to form controls, such as a browse or select
 * button. Menus set on it open on click.
 */
MIconButton::MIconButton(QWidget *parent) : QToolButton(parent)
{
    m_theme = Theme::instance();
    this->setToolButtonStyle(Qt::ToolButtonIconOnly);
    this->setPopupMode(QToolButton::InstantPopup);
    this->setIconSize(m_theme->baseIconSize());
    connect(m_theme, &Theme::themeChanged, this, &MIconButton::refreshTheme);
}

MIconButton::~MIconButton() {}

// Public Methods
// ==============

/**! @brief Set the icon from the icon cache by key name and theme colour.
 *
 * The key and colour are kept so the icon can be regenerated when the theme
 * changes.
 */
void MIconButton::setThemeIcon(const QString &name, ThemeColor color)
{
    m_iconName = name;
    m_iconColor = color;
    this->setIcon(m_theme->icons()->getIcon(name, color, this->iconSize()));
}

/**! @brief A square the height of a standard push button.
 */
QSize MIconButton::sizeHint() const
{
    const int side = qRound(buttonScale * this->fontMetrics().height());
    return QSize(side, side);
}

// Public Slots
// ============

/**! @brief Regenerate the icon after a theme change.
 */
void MIconButton::refreshTheme()
{
    if (!m_iconName.isEmpty()) {
        this->setIcon(m_theme->icons()->getIcon(m_iconName, m_iconColor, this->iconSize()));
    }
}

} // namespace Collett
