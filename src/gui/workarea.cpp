/*
** Collett – GUI Work Area Class
** =============================
**
** This file is a part of Collett
** Copyright 2021–2022, Veronica Berglyd Olsen
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

#include "workarea.h"
#include "doceditor.h"

namespace Collett {

GuiWorkArea::GuiWorkArea(QWidget *parent) : QTabWidget(parent) {

    m_collectionWidget = new GuiCollectionWidget(this);

    this->addTab(m_collectionWidget, tr("Collection"));
}

void GuiWorkArea::openDocument(const QString &path) {

    GuiDocEditor *editor = new GuiDocEditor(this);
    editor->openDocument(path);

    this->addTab(editor, "Document");
}

} // namespace Collett
