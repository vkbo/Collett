/*
** Collett – GUI Story Tree Delegate Class
** =======================================
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

#ifndef GUI_STORYTREEDELEGATE_H
#define GUI_STORYTREEDELEGATE_H

#include <QSize>
#include <QFont>
#include <QObject>
#include <QPainter>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QAbstractItemDelegate>

namespace Collett {

class GuiStoryTreeDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    GuiStoryTreeDelegate(QWidget *parent=nullptr);
    ~GuiStoryTreeDelegate() {};

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QFont m_headFont;
    QFont m_mainFont;
    int m_headHeight;
    int m_mainHeight;

};
} // namespace Collett

#endif // GUI_STORYTREEDELEGATE_H
