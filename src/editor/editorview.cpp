/*
** Collett – GUI Editor View Class
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

#include "collett.h"
#include "editorview.h"
#include "texteditor.h"

#include <QWidget>
#include <QVBoxLayout>

namespace Collett {

// Constructor/Destructor
// ======================

GuiEditorView::GuiEditorView(QWidget *parent) : QWidget(parent) {

    // Components
    textEditor = new GuiTextEditor(this);

    // Assemble
    QVBoxLayout *outerBox = new QVBoxLayout();
    outerBox->addWidget(textEditor, 1);

    this->setLayout(outerBox);
}

GuiEditorView::~GuiEditorView() {
    qDebug() << "Destructor: GuiEditorView";
}

} // namespace Collett
