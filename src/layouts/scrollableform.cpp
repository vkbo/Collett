/*
** Collett - Scrollable Form Layout
** ================================
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
#include "scrollableform.h"

#include <QColor>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QRectF>
#include <QScrollArea>
#include <QSizePolicy>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

namespace Collett {

namespace {
// Space between a group and the next group label
constexpr int groupSpacing = 20;
// Space between the text and the control of a row
constexpr int rowSpacing = 16;
// Padding inside each row
constexpr int rowPaddingH = 12;
constexpr int rowPaddingV = 10;
// Corner radius of the group background
constexpr qreal groupRadius = 6.0;
// Alpha of the text colour used for the divider between rows
constexpr int dividerAlpha = 40;
} // namespace

// Scrollable Form
// ===============

/**! @brief A scrollable page of settings arranged in labelled groups.
 *
 * Rows are added to the most recent group. Adding a group label starts a new
 * group, and rows added before any label go into an unlabelled group.
 */
MScrollableForm::MScrollableForm(QWidget *parent) : QScrollArea(parent)
{
    m_layout = new QVBoxLayout();
    m_layout->setSpacing(4);

    m_content = new QWidget(this);
    m_content->setLayout(m_layout);

    this->setWidget(m_content);
    this->setWidgetResizable(true);
    this->setFrameShape(QFrame::NoFrame);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

MScrollableForm::~MScrollableForm() {}

/**! @brief The total number of rows across all groups.
 */
int MScrollableForm::rowCount() const
{
    int count = 0;
    for (const MSettingsGroup *group : m_groups) {
        count += group->rowCount();
    }
    return count;
}

/**! @brief Add a section label and start a new group below it.
 */
void MScrollableForm::addGroupLabel(const QString &text)
{
    QLabel *label = new QLabel(text, m_content);
    QFont font = label->font();
    font.setBold(true);
    label->setFont(font);
    label->setContentsMargins(0, 4, 0, 4);

    if (!m_groups.isEmpty()) {
        m_layout->addSpacing(groupSpacing);
    }
    m_layout->addWidget(label);

    MSettingsGroup *group = new MSettingsGroup(m_content);
    m_layout->addWidget(group);
    m_groups.append(group);
}

/**! @brief Add a setting to the current group.
 *
 * The label and optional help text fill the space to the left of the widget.
 */
void MScrollableForm::addRow(const QString &label, QWidget *widget, const QString &helpText)
{
    this->currentGroup()->addRow(label, widget, helpText);
}

/**! @brief Push the groups to the top once all rows have been added.
 */
void MScrollableForm::finalise()
{
    m_layout->addStretch(1);
}

/**! @brief The group rows are currently added to, created if there is none.
 */
MSettingsGroup *MScrollableForm::currentGroup()
{
    if (m_groups.isEmpty()) {
        MSettingsGroup *group = new MSettingsGroup(m_content);
        m_layout->addWidget(group);
        m_groups.append(group);
    }
    return m_groups.last();
}

// Settings Group
// ==============

/**! @brief A box of settings rows on a lighter background.
 */
MSettingsGroup::MSettingsGroup(QWidget *parent) : QWidget(parent)
{
    m_layout = new QVBoxLayout();
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    this->setLayout(m_layout);
}

MSettingsGroup::~MSettingsGroup() {}

/**! @brief Add a row with a label, optional help text, and a control.
 *
 * The label is drawn in a demi-bold weight and the help text wraps. Both take
 * the space left over by the control, which is placed on the right.
 */
void MSettingsGroup::addRow(const QString &label, QWidget *widget, const QString &helpText)
{
    QWidget *row = new QWidget(this);

    QLabel *titleLabel = new QLabel(label, row);
    QFont titleFont = titleLabel->font();
    titleFont.setWeight(QFont::DemiBold);
    titleLabel->setFont(titleFont);
    titleLabel->setBuddy(widget);
    widget->setAccessibleName(label);

    QVBoxLayout *textBox = new QVBoxLayout();
    textBox->setContentsMargins(0, 0, 0, 0);
    textBox->setSpacing(2);
    textBox->addWidget(titleLabel);

    if (!helpText.isEmpty()) {
        QLabel *helpLabel = new QLabel(helpText, row);
        helpLabel->setWordWrap(true);
        helpLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        helpLabel->setBuddy(widget);
        widget->setAccessibleDescription(helpText);
        textBox->addWidget(helpLabel);
    }

    QHBoxLayout *rowBox = new QHBoxLayout();
    rowBox->setContentsMargins(rowPaddingH, rowPaddingV, rowPaddingH, rowPaddingV);
    rowBox->setSpacing(rowSpacing);
    rowBox->addLayout(textBox, 1);
    rowBox->addWidget(widget, 0, Qt::AlignRight | Qt::AlignVCenter);
    row->setLayout(rowBox);

    m_layout->addWidget(row);
    m_rows.append(row);
}

/**! @brief Draw the group background and the dividers between rows.
 */
void MSettingsGroup::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    const QPalette palette = this->palette();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(palette.base());
    painter.drawRoundedRect(QRectF(this->rect()), groupRadius, groupRadius);

    QColor divider = palette.text().color();
    divider.setAlpha(dividerAlpha);
    painter.setPen(divider);
    painter.setRenderHint(QPainter::Antialiasing, false);
    for (int i = 0; i < m_rows.count() - 1; ++i) {
        const int y = m_rows.at(i)->geometry().bottom();
        painter.drawLine(rowPaddingH, y, this->width() - rowPaddingH, y);
    }
}

} // namespace Collett
