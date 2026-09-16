/*
** Collett - Modified QToolButton Class
** ====================================
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
#include "mtoolbutton.h"
#include "theme.h"

#include <QColor>
#include <QFontMetrics>
#include <QIcon>
#include <QPainter>
#include <QPainterStateGuard>
#include <QPaintEvent>
#include <QPixmap>
#include <QRect>
#include <QSize>
#include <QStyle>
#include <QStyleOptionToolButton>
#include <QToolButton>
#include <QWidget>

namespace Collett {

namespace {
// Alpha of the highlight colour used for the background tint
constexpr int hoverAlpha = 60;
constexpr int activeAlpha = 120;
} // namespace

// Constructor/Destructor
// ======================

MToolButton::MToolButton(QWidget *parent) : QToolButton(parent)
{
    m_theme = Theme::instance();
    m_buttonSize = m_theme->toolButtonSize();
    this->updateIconSize();
    connect(m_theme, &Theme::themeChanged, this, &MToolButton::refreshTheme);
}

MToolButton::~MToolButton()
{
    qDebug() << "Destructor: MToolButton";
}

// Setters
// =======

/**! @brief Set the size of the button.
 *
 * The icon size is derived from the button size and the padding.
 */
void MToolButton::setButtonSize(const QSize &size)
{
    m_buttonSize = size;
    this->updateIconSize();
}

/**! @brief Set the inner padding as a fraction of the button size.
 *
 * The padding is applied on all sides, so the icon is the button size times
 * one minus twice the padding.
 */
void MToolButton::setPadding(qreal padding)
{
    m_padding = qBound(0.0, padding, 0.5);
    this->updateIconSize();
}

// Public Methods
// ==============

/**! @brief Set the icon from the icon cache by key name and theme colour.
 *
 * The icon is rendered at the button's current icon size, so set the size
 * before calling this. The key and colour are kept so the icon can be
 * regenerated when the theme changes. If the button has a default action,
 * the icon is set on the action, which the button then picks up.
 */
void MToolButton::setThemeIcon(const QString &name, ThemeColor color)
{
    m_iconName = name;
    m_iconColor = color;
    QIcon icon = m_theme->icons()->getIcon(name, color, this->iconSize());
    if (QAction *action = this->defaultAction()) {
        action->setIcon(icon);
    } else {
        this->setIcon(icon);
    }
}

/**! @brief The fixed button size for icon-only buttons.
 *
 * Buttons with text fall back to the default size hint.
 */
QSize MToolButton::sizeHint() const
{
    if (this->toolButtonStyle() != Qt::ToolButtonIconOnly) {
        return QToolButton::sizeHint();
    }
    return m_buttonSize;
}

// Public Slots
// ============

/**! @brief Regenerate the icon after a theme change.
 */
void MToolButton::refreshTheme()
{
    if (!m_iconName.isEmpty()) {
        this->setThemeIcon(m_iconName, m_iconColor);
    }
}

// Private Helpers
// ===============

/**! @brief Recompute the icon size from the button size and padding.
 *
 * The theme icon, if any, is regenerated at the new size.
 */
void MToolButton::updateIconSize()
{
    const qreal scale = 1.0 - 2.0 * m_padding;
    const int width = qMax(1, qRound(m_buttonSize.width() * scale));
    const int height = qMax(1, qRound(m_buttonSize.height() * scale));
    this->setIconSize(QSize(width, height));
    this->refreshTheme();
    this->updateGeometry();
}

// Events
// ======

/**! @brief Paint the tool button.
 *
 * The button is flat. The background is tinted with the highlight colour when
 * the button is hovered or has keyboard focus, and tinted more strongly while
 * it is checked, pressed, or its menu is open.
 */
void MToolButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    QStyleOptionToolButton option;
    this->initStyleOption(&option);

    // The style option takes the icon size from a parent tool bar, but the
    // button's own icon size is the one derived from the padding
    option.iconSize = this->iconSize();

    const bool isEnabled = option.state & QStyle::State_Enabled;
    const bool isActive = option.state & (QStyle::State_On | QStyle::State_Sunken);
    const bool isHovered = isEnabled && (option.state & (QStyle::State_MouseOver | QStyle::State_HasFocus));

    // Background
    if (isActive || isHovered) {
        QColor fill = option.palette.highlight().color();
        fill.setAlpha(isActive ? activeAlpha : hoverAlpha);
        painter.fillRect(option.rect, fill);
    }

    // Label
    this->drawLabel(&painter, option, option.rect, option.state);
}

// Painters
// ========

/**! @brief Draw the icon and text of the button.
 *
 * Mirrors QCommonStyle's CE_ToolButtonLabel. No offset is applied when the
 * button is pressed.
 */
void MToolButton::drawLabel(QPainter *painter, const QStyleOptionToolButton &option, const QRect &rect, QStyle::State state)
{
    const bool isEnabled = state & QStyle::State_Enabled;
    const bool hasIcon = !option.icon.isNull();
    const bool textOnly = option.toolButtonStyle == Qt::ToolButtonTextOnly || (!hasIcon && !option.text.isEmpty());

    int alignment = Qt::TextShowMnemonic;
    if (!this->style()->styleHint(QStyle::SH_UnderlineShortcut, &option, this)) {
        alignment |= Qt::TextHideMnemonic;
    }

    QPainterStateGuard guard(painter);
    painter->setFont(option.font);

    if (textOnly) {
        alignment |= Qt::AlignCenter;
        const QString text = option.fontMetrics.elidedText(option.text, Qt::ElideRight, rect.width(), alignment);
        this->style()->drawItemText(painter, rect, alignment, option.palette, isEnabled, text, QPalette::ButtonText);
        return;
    }

    // Resolve the icon pixmap
    QPixmap pixmap;
    QSize pixmapSize = option.iconSize;
    if (!option.icon.isNull()) {
        const QIcon::State iconState = (state & QStyle::State_On) ? QIcon::On : QIcon::Off;
        QIcon::Mode iconMode = QIcon::Normal;
        if (!isEnabled) {
            iconMode = QIcon::Disabled;
        } else if ((state & QStyle::State_MouseOver) && (state & QStyle::State_AutoRaise)) {
            iconMode = QIcon::Active;
        }
        pixmap = option.icon.pixmap(option.rect.size().boundedTo(option.iconSize), painter->device()->devicePixelRatio(), iconMode, iconState);
        pixmapSize = pixmap.size() / pixmap.devicePixelRatio();
    }

    if (option.toolButtonStyle == Qt::ToolButtonIconOnly) {
        this->style()->drawItemPixmap(painter, rect, Qt::AlignCenter, pixmap);
        return;
    }

    // Icon with text beside or under it
    QRect pr = rect;
    QRect tr = rect;
    if (option.toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
        pr.setHeight(pixmapSize.height() + 4);
        tr.adjust(0, pr.height() - 1, 0, -1);
        alignment |= Qt::AlignCenter;
    } else {
        pr.setWidth(pixmapSize.width() + 4);
        tr.adjust(pr.width(), 0, 0, 0);
        pr = QStyle::visualRect(option.direction, rect, pr);
        alignment |= Qt::AlignLeft | Qt::AlignVCenter;
    }

    this->style()->drawItemPixmap(painter, pr, Qt::AlignCenter, pixmap);

    const QString text = option.fontMetrics.elidedText(option.text, Qt::ElideRight, tr.width(), alignment);
    this->style()->drawItemText(
        painter, QStyle::visualRect(option.direction, rect, tr), alignment, option.palette, isEnabled, text, QPalette::ButtonText
    );
}

} // namespace Collett
