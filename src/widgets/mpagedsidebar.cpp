/*
** Collett - Paged Side Bar Widget
** ===============================
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
#include "mpagedsidebar.h"

#include <QAbstractButton>
#include <QAction>
#include <QButtonGroup>
#include <QColor>
#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QPoint>
#include <QPolygon>
#include <QRectF>
#include <QSize>
#include <QSizePolicy>
#include <QString>
#include <QStyle>
#include <QStyleOptionToolButton>
#include <QToolBar>
#include <QToolButton>
#include <QWidget>

namespace Collett {

namespace {
// Height of buttons and labels relative to the font height
constexpr qreal rowScale = 1.7;
// Corner radius of the button background
constexpr qreal cornerRadius = 4.0;
// Alpha of the highlight colour used for the hover tint
constexpr int hoverAlpha = 60;
// Opacity of the highlight colour used for the checked background
constexpr qreal checkedOpacity = 0.8;
// Alpha of the text colour used for the arrow on unchecked buttons
constexpr int arrowAlpha = 64;
// Horizontal text offset in buttons and labels
constexpr int buttonIndent = 12;
constexpr int labelIndent = 4;
} // namespace

// Side Bar
// ========

/**! @brief A vertical bar of labels and buttons that act as tabs.
 *
 * It is meant to be paired with a stacked widget. The buttons form an
 * exclusive group, and clicking one emits its id.
 */
MPagedSideBar::MPagedSideBar(QWidget *parent) : QToolBar(parent)
{
    m_labelColor = this->palette().text().color();

    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);
    connect(m_group, &QButtonGroup::buttonClicked, this, &MPagedSideBar::onButtonClicked);

    this->setMovable(false);
    this->setOrientation(Qt::Vertical);

    // Labels and buttons are inserted in front of this stretch, which keeps
    // them packed at the top of the bar
    QWidget *stretch = new QWidget(this);
    stretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_stretchAction = this->addWidget(stretch);
}

MPagedSideBar::~MPagedSideBar() {}

/**! @brief Return the button with the given id, or null if there is none.
 */
MPagedToolButton *MPagedSideBar::button(int buttonId) const
{
    return qobject_cast<MPagedToolButton *>(m_group->button(buttonId));
}

/**! @brief Set the text colour of all current and future labels.
 */
void MPagedSideBar::setLabelColor(const QColor &color)
{
    m_labelColor = color;
    for (MPagedToolLabel *label : std::as_const(m_labels)) {
        label->setTextColor(color);
    }
}

/**! @brief Check the button with the given id without emitting a click.
 */
void MPagedSideBar::setSelected(int buttonId)
{
    if (QAbstractButton *button = m_group->button(buttonId)) {
        button->setChecked(true);
    }
}

/**! @brief Add a section label above the buttons that follow it.
 */
void MPagedSideBar::addLabel(const QString &text)
{
    MPagedToolLabel *label = new MPagedToolLabel(text, this);
    label->setTextColor(m_labelColor);
    m_labels.append(label);
    this->insertWidget(m_stretchAction, label);
}

/**! @brief Add a button with the given id.
 *
 * The id is what the buttonClicked signal reports, and is normally the index
 * of the matching page in the stacked widget.
 */
void MPagedSideBar::addButton(const QString &text, int buttonId)
{
    MPagedToolButton *button = new MPagedToolButton(this);
    button->setText(text);
    button->setAccessibleName(text);
    this->insertWidget(m_stretchAction, button);
    m_group->addButton(button, buttonId);
}

/**! @brief Forward a click in the button group as the button's id.
 */
void MPagedSideBar::onButtonClicked(QAbstractButton *button)
{
    const int buttonId = m_group->id(button);
    if (buttonId != -1) {
        emit buttonClicked(buttonId);
    }
}

// Tool Button
// ===========

MPagedToolButton::MPagedToolButton(QWidget *parent) : QToolButton(parent)
{
    const int fontHeight = this->fontMetrics().height();
    m_buttonHeight = qRound(rowScale * fontHeight);
    m_textMargin = (m_buttonHeight - fontHeight) / 2;
    m_arrowSize = 2 * fontHeight / 7;

    this->setCheckable(true);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    this->setFixedHeight(m_buttonHeight);
}

MPagedToolButton::~MPagedToolButton() {}

/**! @brief The default size hint, widened to make room for the arrow.
 */
QSize MPagedToolButton::sizeHint() const
{
    return QToolButton::sizeHint() + QSize(4 * m_arrowSize, 0);
}

/**! @brief Draw a left aligned text label with an arrow on the right.
 *
 * The background is tinted with the highlight colour when hovered, and filled
 * with it when checked.
 */
void MPagedToolButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QStyleOptionToolButton option;
    option.initFrom(this);

    const int width = this->width();
    const int height = this->height();
    const QPalette palette = this->palette();
    const bool isChecked = this->isChecked();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);

    if (option.state & QStyle::State_MouseOver) {
        QColor fill = palette.highlight().color();
        fill.setAlpha(hoverAlpha);
        painter.setBrush(fill);
        painter.drawRoundedRect(0, 0, width, height, cornerRadius, cornerRadius);
    }

    if (isChecked) {
        painter.setBrush(palette.highlight());
        painter.setOpacity(checkedOpacity);
        painter.drawRoundedRect(0, 0, width, height, cornerRadius, cornerRadius);
        painter.setOpacity(1.0);
    }

    const QColor textColor = isChecked ? palette.highlightedText().color() : palette.text().color();
    const QRectF textRect(buttonIndent, m_textMargin, width - 2 * buttonIndent, height - 2 * m_textMargin);
    painter.setPen(textColor);
    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, this->text());

    QColor arrowColor = textColor;
    if (!isChecked) {
        arrowColor.setAlpha(arrowAlpha);
    }

    const int arrowX = width - m_arrowSize - buttonIndent;
    const int arrowY = height / 2;
    painter.setPen(Qt::NoPen);
    painter.setBrush(arrowColor);
    painter.drawPolygon(QPolygon({
        QPoint(arrowX, arrowY - m_arrowSize),
        QPoint(arrowX + m_arrowSize, arrowY),
        QPoint(arrowX, arrowY + m_arrowSize),
    }));
}

// Tool Label
// ==========

MPagedToolLabel::MPagedToolLabel(const QString &text, QWidget *parent) : QLabel(text, parent)
{
    const int fontHeight = this->fontMetrics().height();
    const int labelHeight = qRound(rowScale * fontHeight);
    m_textMargin = (labelHeight - fontHeight) / 2;
    m_textColor = this->palette().text().color();

    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    this->setFixedHeight(labelHeight);
}

MPagedToolLabel::~MPagedToolLabel() {}

/**! @brief Set the text colour and repaint.
 */
void MPagedToolLabel::setTextColor(const QColor &color)
{
    m_textColor = color;
    this->update();
}

/**! @brief Draw a left aligned text label matching the button layout.
 */
void MPagedToolLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    const QRectF textRect(labelIndent, m_textMargin, this->width() - 2 * labelIndent, this->height() - 2 * m_textMargin);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(m_textColor);
    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, this->text());
}

} // namespace Collett
