/*
** Collett - GUI Text Editor Class
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
#include "texteditor.h"

#include <QFont>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextEdit>

namespace Collett {

// Constructor/Destructor
// ======================

GuiTextEditor::GuiTextEditor(QWidget *parent) : QTextEdit(parent)
{
}

GuiTextEditor::~GuiTextEditor()
{
    qDebug() << "Destructor: GuiTextEditor";
}

// Public Methods
// ==============

void GuiTextEditor::openDocument(Document *doc)
{
    this->setDocument(doc);
    this->setEnabled(doc != nullptr);
}

// Public Slots
// ============

void GuiTextEditor::toggleBold(bool bold)
{
    QTextCharFormat format;
    format.setFontWeight(bold ? QFont::Bold : QFont::Normal);
    this->mergeFormatOnWordOrSelection(format);
}

void GuiTextEditor::toggleItalic(bool italic)
{
    QTextCharFormat format;
    format.setFontItalic(italic);
    this->mergeFormatOnWordOrSelection(format);
}

void GuiTextEditor::toggleUnderline(bool underline)
{
    QTextCharFormat format;
    format.setFontUnderline(underline);
    this->mergeFormatOnWordOrSelection(format);
}

void GuiTextEditor::toggleStrikeOut(bool strikeOut)
{
    QTextCharFormat format;
    format.setFontStrikeOut(strikeOut);
    this->mergeFormatOnWordOrSelection(format);
}

void GuiTextEditor::toggleSuperscript(bool superscript)
{
    QTextCharFormat format;
    format.setVerticalAlignment(superscript ? QTextCharFormat::AlignSuperScript : QTextCharFormat::AlignNormal);
    this->mergeFormatOnWordOrSelection(format);
}

void GuiTextEditor::toggleSubscript(bool subscript)
{
    QTextCharFormat format;
    format.setVerticalAlignment(subscript ? QTextCharFormat::AlignSubScript : QTextCharFormat::AlignNormal);
    this->mergeFormatOnWordOrSelection(format);
}

void GuiTextEditor::alignLeft()
{
    this->setAlignment(Qt::AlignLeft);
}

void GuiTextEditor::alignCenter()
{
    this->setAlignment(Qt::AlignCenter);
}

void GuiTextEditor::alignRight()
{
    this->setAlignment(Qt::AlignRight);
}

void GuiTextEditor::alignJustify()
{
    this->setAlignment(Qt::AlignJustify);
}

// Private Methods
// ===============

void GuiTextEditor::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = this->textCursor();
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }
    cursor.mergeCharFormat(format);
    this->mergeCurrentCharFormat(format);
}

} // namespace Collett
