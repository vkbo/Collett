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

#include <QColor>
#include <QFontMetrics>
#include <QIcon>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterStateGuard>
#include <QPaintEvent>
#include <QPixmap>
#include <QPointF>
#include <QRect>
#include <QRectF>
#include <QStyle>
#include <QStyleOptionToolButton>
#include <QToolButton>
#include <QWidget>

namespace Collett {

namespace {
// Colours and metrics lifted from QFusionStyle
constexpr QColor innerContrastLine = QColor(255, 255, 255, 30);
constexpr int frameWidth = 1;
constexpr int baseMenuIndicator = 12;
constexpr int baseArrowWidth = 14;
constexpr int baseArrowHeight = 8;
} // namespace

// Constructor/Destructor
// ======================

MToolButton::MToolButton(QWidget *parent) : QToolButton(parent)
{
}

MToolButton::~MToolButton()
{
    qDebug() << "Destructor: MToolButton";
}

// Events
// ======

/**! @brief Paint the tool button.
 *
 * Reproduces the QFusionStyle rendering of a tool button, which is composed of
 * QCommonStyle::drawComplexControl for CC_ToolButton and the Fusion primitives
 * it calls into. The panel is only drawn when the button is raised, sunken or
 * checked, so auto-raise buttons are flat until hovered.
 */
void MToolButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    QStyleOptionToolButton option;
    this->initStyleOption(&option);

    const bool hasMenuButton = option.features & QStyleOptionToolButton::MenuButtonPopup;
    const int mbi = this->dpiScaled(baseMenuIndicator);

    QRect buttonRect = option.rect;
    QRect menuRect = option.rect;
    if (hasMenuButton) {
        buttonRect.adjust(0, 0, -mbi, 0);
        menuRect.adjust(menuRect.width() - mbi, 0, 0, 0);
    }

    // Resolve the state flags for the button and menu areas
    QStyle::State bflags = option.state & ~QStyle::State_Sunken;
    if (bflags & QStyle::State_AutoRaise) {
        if (!(bflags & QStyle::State_MouseOver) || !(bflags & QStyle::State_Enabled)) {
            bflags &= ~QStyle::State_Raised;
        }
    }
    QStyle::State mflags = bflags;
    if (option.state & QStyle::State_Sunken) {
        if (option.activeSubControls & QStyle::SC_ToolButton) {
            bflags |= QStyle::State_Sunken;
        }
        mflags |= QStyle::State_Sunken;
    }

    // Button panel
    if (option.subControls & QStyle::SC_ToolButton) {
        if (bflags & (QStyle::State_Sunken | QStyle::State_On | QStyle::State_Raised)) {
            this->drawPanel(&painter, option, buttonRect, bflags);
        }
    }

    // Focus frame
    if (option.state & QStyle::State_HasFocus) {
        QRect focusRect = option.rect.adjusted(3, 3, -3, -3);
        if (hasMenuButton) {
            focusRect.adjust(0, 0, -mbi, 0);
        }
        this->drawFocusRect(&painter, option, focusRect);
    }

    // Label
    this->drawLabel(&painter, option, buttonRect.adjusted(frameWidth, frameWidth, -frameWidth, -frameWidth), bflags);

    // Menu indicator
    if (option.subControls & QStyle::SC_ToolButtonMenu) {
        if (mflags & (QStyle::State_Sunken | QStyle::State_On | QStyle::State_Raised)) {
            this->drawPanel(&painter, option, menuRect, mflags);
        }
        this->drawArrow(&painter, option, menuRect);
    } else if (option.features & QStyleOptionToolButton::HasMenu) {
        QRect ir = option.rect;
        QRect arrowRect(ir.right() + 5 - mbi, ir.y() + ir.height() - mbi + 4, mbi - 6, mbi - 6);
        this->drawArrow(&painter, option, QStyle::visualRect(option.direction, buttonRect, arrowRect));
    }
}

// Painters
// ========

/**! @brief Draw the raised, sunken or checked button panel.
 *
 * Mirrors QFusionStyle's PE_PanelButtonCommand as reached through
 * PE_PanelButtonTool. The default and flat button branches do not apply to
 * tool buttons and are left out.
 */
void MToolButton::drawPanel(QPainter *painter, const QStyleOptionToolButton &option, const QRect &rect, QStyle::State state)
{
    const bool isDown = (state & QStyle::State_Sunken) || (state & QStyle::State_On);
    const bool isEnabled = state & QStyle::State_Enabled;
    const bool hasFocus = (state & QStyle::State_HasFocus) && (state & QStyle::State_KeyboardFocusChange);
    const bool isHovered = isEnabled && (state & QStyle::State_MouseOver);

    const QColor button = this->buttonColor(option.palette);
    const QColor darkOutline = hasFocus ? this->highlightedOutlineColor(option.palette) : this->outlineColor(option.palette);
    const QRect r = rect.adjusted(0, 1, -1, 0);

    QPainterStateGuard guard(painter);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->translate(0.5, -0.5);

    // Fill
    QLinearGradient gradient = this->panelGradient(rect, isHovered ? button : button.darker(104));
    painter->setPen(Qt::transparent);
    painter->setBrush(isDown ? QBrush(button.darker(110)) : QBrush(gradient));
    painter->drawRoundedRect(r, 2.0, 2.0);
    painter->setBrush(Qt::NoBrush);

    // Outline
    painter->setPen(isEnabled ? darkOutline : darkOutline.lighter(115));
    painter->drawRoundedRect(r, 2.0, 2.0);

    // Inner contrast line
    painter->setPen(innerContrastLine);
    painter->drawRoundedRect(r.adjusted(1, 1, -1, -1), 2.0, 2.0);
}

/**! @brief Draw the keyboard focus frame.
 *
 * Mirrors QFusionStyle's PE_FrameFocusRect, which is only visible after a
 * keyboard driven focus change.
 */
void MToolButton::drawFocusRect(QPainter *painter, const QStyleOptionToolButton &option, const QRect &rect)
{
    if (!(option.state & QStyle::State_KeyboardFocusChange)) {
        return;
    }

    QPainterStateGuard guard(painter);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->translate(0.5, 0.5);

    QColor fillColor = this->highlightedOutlineColor(option.palette);
    fillColor.setAlpha(80);
    painter->setPen(fillColor.darker(120));
    fillColor.setAlpha(30);

    QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
    gradient.setColorAt(0, fillColor.lighter(160));
    gradient.setColorAt(1, fillColor);
    painter->setBrush(gradient);
    painter->drawRoundedRect(rect.adjusted(0, 0, -1, -1), 1, 1);
}

/**! @brief Draw the icon and text of the button.
 *
 * Mirrors QCommonStyle's CE_ToolButtonLabel, which Fusion does not override.
 * Fusion sets the button shift metrics to zero, so no offset is applied when
 * the button is pressed.
 */
void MToolButton::drawLabel(QPainter *painter, const QStyleOptionToolButton &option, const QRect &rect, QStyle::State state)
{
    const bool isEnabled = state & QStyle::State_Enabled;
    const bool hasArrow = option.features & QStyleOptionToolButton::Arrow;
    const bool hasIcon = !option.icon.isNull() || hasArrow;
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
        if (hasArrow) {
            this->drawArrow(painter, option, rect);
        } else {
            this->style()->drawItemPixmap(painter, rect, Qt::AlignCenter, pixmap);
        }
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

    if (hasArrow) {
        this->drawArrow(painter, option, pr);
    } else {
        this->style()->drawItemPixmap(painter, pr, Qt::AlignCenter, pixmap);
    }

    const QString text = option.fontMetrics.elidedText(option.text, Qt::ElideRight, tr.width(), alignment);
    this->style()->drawItemText(
        painter, QStyle::visualRect(option.direction, rect, tr), alignment, option.palette, isEnabled, text, QPalette::ButtonText
    );
}

/**! @brief Draw a down arrow centred in the given rectangle.
 *
 * Mirrors qt_fusion_draw_arrow as used for PE_IndicatorArrowDown. When the
 * button has an explicit arrow type, that direction is used instead.
 */
void MToolButton::drawArrow(QPainter *painter, const QStyleOptionToolButton &option, const QRect &rect)
{
    if (rect.isEmpty() || rect.width() <= 1 || rect.height() <= 1) {
        return;
    }

    Qt::ArrowType type = Qt::DownArrow;
    if (option.features & QStyleOptionToolButton::Arrow) {
        type = option.arrowType;
    }

    QColor arrowColor = option.palette.windowText().color();
    arrowColor.setAlpha(160);

    const int arrowWidth = this->dpiScaled(baseArrowWidth);
    const int arrowHeight = this->dpiScaled(baseArrowHeight);
    const int arrowMax = qMin(arrowHeight, arrowWidth);
    const int rectMax = qMin(rect.height(), rect.width());
    const int size = qMin(arrowMax, rectMax);

    QRectF arrowRect(0, 0, size, arrowHeight * size / arrowWidth);
    if (type == Qt::LeftArrow || type == Qt::RightArrow) {
        arrowRect = arrowRect.transposed();
    }
    arrowRect.moveTo(
        rect.x() + (rect.width() - arrowRect.width()) / 2.0,
        rect.y() + (rect.height() - arrowRect.height()) / 2.0
    );

    QPointF triangle[3];
    switch (type) {
    case Qt::DownArrow:
        triangle[0] = arrowRect.topLeft();
        triangle[1] = arrowRect.topRight();
        triangle[2] = QPointF(arrowRect.center().x(), arrowRect.bottom());
        break;
    case Qt::RightArrow:
        triangle[0] = arrowRect.topLeft();
        triangle[1] = arrowRect.bottomLeft();
        triangle[2] = QPointF(arrowRect.right(), arrowRect.center().y());
        break;
    case Qt::LeftArrow:
        triangle[0] = arrowRect.topRight();
        triangle[1] = arrowRect.bottomRight();
        triangle[2] = QPointF(arrowRect.left(), arrowRect.center().y());
        break;
    default:
        triangle[0] = arrowRect.bottomLeft();
        triangle[1] = arrowRect.bottomRight();
        triangle[2] = QPointF(arrowRect.center().x(), arrowRect.top());
        break;
    }

    QPainterStateGuard guard(painter);
    painter->setPen(Qt::NoPen);
    painter->setBrush(arrowColor);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->drawPolygon(triangle, 3);
}

// Colours
// =======

/**! @brief The Fusion frame outline colour.
 */
QColor MToolButton::outlineColor(const QPalette &palette) const
{
    if (palette.window().style() == Qt::TexturePattern) {
        return QColor(0, 0, 0, 160);
    }
    QColor windowColor = palette.window().color();
    if (!windowColor.isValid()) {
        windowColor = QPalette().window().color();
    }
    return windowColor.darker(140);
}

/**! @brief The Fusion outline colour for focused elements.
 */
QColor MToolButton::highlightedOutlineColor(const QPalette &palette) const
{
    QColor highlighted = palette.highlight().color().darker(125);
    if (highlighted.value() > 160) {
        highlighted.setHsl(highlighted.hue(), highlighted.saturation(), 160);
    }
    return highlighted;
}

/**! @brief The Fusion base colour for button panels.
 */
QColor MToolButton::buttonColor(const QPalette &palette) const
{
    QColor button = palette.button().color();
    const int val = qGray(button.rgb());
    button = button.lighter(100 + qMax(1, (180 - val) / 6));
    button.setHsv(button.hue(), button.saturation() * 0.75, button.value(), button.alpha());
    return button;
}

/**! @brief The Fusion top-down panel gradient.
 */
QLinearGradient MToolButton::panelGradient(const QRect &rect, const QColor &baseColor) const
{
    const int x = rect.center().x();
    QLinearGradient gradient(x, rect.top(), x, rect.bottom());
    gradient.setColorAt(0, baseColor.lighter(124));
    gradient.setColorAt(1, baseColor.lighter(102));
    return gradient;
}

// Helpers
// =======

/**! @brief Scale a 96 dpi pixel value to the widget's logical dpi.
 */
int MToolButton::dpiScaled(int value) const
{
    return int(value * this->logicalDpiX() / 96.0);
}

} // namespace Collett
