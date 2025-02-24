/*
** Collett – Modified QTreeView Class
** ==================================
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

#ifndef COLLETT_MOD_TREE_VIEW_H
#define COLLETT_MOD_TREE_VIEW_H

#include "collett.h"

#include <QTreeView>
#include <QWidget>
#include <QModelIndex>

namespace Collett {

class MTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit MTreeView(QWidget *parent = nullptr);
    ~MTreeView();

    // Events
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void middleClicked(const QModelIndex &index);

};
} // namespace Collett

#endif // COLLETT_MOD_TREE_VIEW_H
