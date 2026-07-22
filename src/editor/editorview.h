/*
** Collett - GUI Editor View Class
** ===============================
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

#pragma once

#include "collett.h"
#include "texteditor.h"

#include <QWidget>

namespace Collett {

class GuiEditorView : public QWidget
{
    Q_OBJECT

public:
    explicit GuiEditorView(QWidget *parent = nullptr);
    ~GuiEditorView();

    GuiTextEditor *textEditor = nullptr;
};
} // namespace Collett
