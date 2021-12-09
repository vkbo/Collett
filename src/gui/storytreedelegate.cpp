/*
** Collett – GUI Story Tree Delegate Class
** =======================================
**
** This file is a part of Collett
** Copyright 2020–2021, Veronica Berglyd Olsen
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

#include "storytreedelegate.h"

#include <QFont>
#include <QRect>
#include <QSize>
#include <QLocale>
#include <QObject>
#include <QPainter>
#include <QPalette>
#include <QModelIndex>
#include <QApplication>
#include <QFontMetrics>
#include <QStyleOptionViewItem>
#include <QAbstractItemDelegate>

namespace Collett {

GuiStoryTreeDelegate::GuiStoryTreeDelegate(QWidget *parent)
    : QAbstractItemDelegate(parent)
{
    m_headFont = qApp->font();
    m_mainFont = qApp->font();
    m_headFont.setPointSizeF(0.7*m_mainFont.pointSizeF());
    m_headHeight = QFontMetrics(m_headFont).height();
    m_mainHeight = QFontMetrics(m_mainFont).height();
}

void GuiStoryTreeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

    QVariantList data = index.data(Qt::DisplayRole).toList();
    Q_ASSERT(data.size() == 3);

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, qApp->palette().color(QPalette::Highlight));
    }

    QLocale loc;

    QRect rectT = option.rect.adjusted(0, 2, -2, m_headHeight);
    QRect rectW = option.rect.adjusted(0, 2, -2, m_headHeight);
    QRect rectL = option.rect.adjusted(0, m_headHeight + 2, -2, m_mainHeight);

    QFont typeFont = m_headFont;
    typeFont.setWeight(QFont::Bold);

    QString textT = data.at(2).toString();
    QString textW = loc.toString(data.at(1).toInt());
    QString textL = QFontMetrics(m_mainFont).elidedText(data.at(0).toString(), Qt::ElideRight, option.rect.width());

    painter->save();

    painter->setFont(typeFont);
    painter->drawText(rectT, Qt::TextSingleLine, textT);

    painter->setFont(m_headFont);
    painter->drawText(rectW, Qt::TextSingleLine | Qt::AlignRight, textW);

    painter->setFont(m_mainFont);
    painter->drawText(rectL, Qt::TextSingleLine, textL);

    painter->restore();
}

QSize GuiStoryTreeDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return QSize(option.rect.width(), m_headHeight + m_mainHeight + 4);
}

} // namespace Collett
