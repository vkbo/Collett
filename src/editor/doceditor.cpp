/*
** Collett – GUI Document Editor Class
** ===================================
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

#include "collett.h"
#include "doceditor.h"
#include "textedit.h"
#include "edittoolbar.h"

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>

namespace Collett {

GuiDocEditor::GuiDocEditor(QWidget *parent)
    : QWidget(parent)
{
    m_textArea = new GuiTextEdit(this);
    m_editToolBar = new GuiEditToolBar(this);
    connect(m_editToolBar, SIGNAL(documentAction(DocAction)), m_textArea, SLOT(applyDocAction(DocAction)));

    QVBoxLayout *outerBox = new QVBoxLayout;
    outerBox->addWidget(m_editToolBar);
    outerBox->addWidget(m_textArea);
    outerBox->setContentsMargins(0, 0, 0, 0);
    outerBox->setSpacing(0);

    this->setLayout(outerBox);
    m_textArea->setHtml("<b>Hello World</b>");
    qDebug() << "QTextDocument:" << sizeof(m_textArea->document());
}

} // namespace Collett
