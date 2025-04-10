/*
** Collett – Edit Item Dialog
** ==========================
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

#ifndef COLLETT_EDIT_ITEM_H
#define COLLETT_EDIT_ITEM_H

#include "collett.h"

#include <QDialog>
#include <QLineEdit>
#include <QWidget>

namespace Collett {

class Node;
class EditItemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditItemDialog(QWidget *parent, Node *node);
    ~EditItemDialog();

    static void editNode(QWidget *parent, Node *node);

private:
    QLineEdit *m_titleValue;

};
} // namespace Collett

#endif // COLLETT_EDIT_ITEM_H
