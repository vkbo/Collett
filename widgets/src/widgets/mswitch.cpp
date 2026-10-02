/*
** Collett - Toggle Switch Widget
** ==============================
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
#include "mswitch.h"
#include "theme.h"

#include <QAbstractButton>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QPropertyAnimation>
#include <QSizePolicy>
#include <QWidget>

namespace Collett {

namespace {
// Duration of the knob slide when the switch is clicked
constexpr int animDuration = 120;
// Gap between the knob and the edge of the track
constexpr int knobMargin = 2;
} // namespace

// Constructor/Destructor
// ======================

/**! @brief Create a toggle switch.
 *
 * The switch is twice as wide as it is tall. The height defaults to the
 * theme's base button height.
 */
MSwitch::MSwitch(QWidget *parent, int height) : QAbstractButton(parent)
{
    m_height = height > 0 ? height : Theme::instance()->baseButtonHeight();
    m_width = 2 * m_height;
    m_radius = m_height / 2;
    m_knobSize = m_height - 2 * knobMargin;
    m_knobRadius = m_radius - knobMargin;
    m_offset = m_radius;

    m_anim = new QPropertyAnimation(this, "offset", this);
    m_anim->setDuration(animDuration);

    this->setCheckable(true);
    this->setCursor(Qt::PointingHandCursor);
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->setFixedSize(m_width, m_height);
}

MSwitch::~MSwitch() {}

// Setters
// =======

/**! @brief Set the horizontal centre of the knob and repaint.
 *
 * This is the property driven by the slide animation.
 */
void MSwitch::setOffset(int offset)
{
    m_offset = offset;
    this->update();
}

// Events
// ======

/**! @brief Draw the track and the knob.
 *
 * The track is filled with the accent colour when checked, and outlined with
 * the highlight colour when the switch has focus. A disabled switch is drawn
 * at half opacity.
 */
void MSwitch::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    const QPalette palette = this->palette();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setOpacity(this->isEnabled() ? 1.0 : 0.5);

    painter.setPen(this->hasFocus() ? palette.highlight().color() : palette.mid().color());
    painter.setBrush(this->isChecked() ? palette.accent() : palette.alternateBase());
    painter.drawRoundedRect(0, 0, m_width, m_height, m_radius, m_radius);

    painter.setPen(Qt::NoPen);
    painter.setBrush(palette.highlightedText());
    painter.drawEllipse(m_offset - m_knobRadius, knobMargin, m_knobSize, m_knobSize);
}

/**! @brief Move the knob immediately when the state is set programmatically.
 *
 * This is not called for clicks, which go through nextCheckState instead.
 */
void MSwitch::checkStateSet()
{
    m_anim->stop();
    this->setOffset(this->targetOffset());
}

/**! @brief Toggle the state on click and slide the knob to its new position.
 */
void MSwitch::nextCheckState()
{
    QAbstractButton::nextCheckState();
    m_anim->stop();
    m_anim->setStartValue(m_offset);
    m_anim->setEndValue(this->targetOffset());
    m_anim->start();
}

} // namespace Collett
