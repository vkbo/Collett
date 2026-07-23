/*
** Collett - GUI Editor ToolBar
** ============================
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
#include "editortoolbar.h"
#include "texteditor.h"
#include "theme.h"

#include <QAction>
#include <QKeySequence>
#include <QSize>
#include <QTextCharFormat>
#include <QTextEdit>
#include <QToolBar>

namespace Collett {

// Constructor/Destructor
// ======================

GuiEditorToolBar::GuiEditorToolBar(GuiTextEditor *editor, QWidget *parent)
    : QToolBar(parent), m_editor(editor)
{
    m_theme = Theme::instance();
    QSize size = m_theme->toolButtonIconSize();
    this->setIconSize(size);

    actBold = this->addAction(m_theme->icons()->getIcon("fmt_bold", ThemeColor::DefaultColor, size), tr("Bold"));
    actBold->setCheckable(true);
    actBold->setShortcut(QKeySequence::Bold);
    connect(actBold, &QAction::toggled, m_editor, &GuiTextEditor::toggleBold);

    actItalic = this->addAction(m_theme->icons()->getIcon("fmt_italic", ThemeColor::DefaultColor, size), tr("Italic"));
    actItalic->setCheckable(true);
    actItalic->setShortcut(QKeySequence::Italic);
    connect(actItalic, &QAction::toggled, m_editor, &GuiTextEditor::toggleItalic);

    actUnderline = this->addAction(m_theme->icons()->getIcon("fmt_underline", ThemeColor::DefaultColor, size), tr("Underline"));
    actUnderline->setCheckable(true);
    actUnderline->setShortcut(QKeySequence::Underline);
    connect(actUnderline, &QAction::toggled, m_editor, &GuiTextEditor::toggleUnderline);

    actStrikethrough = this->addAction(m_theme->icons()->getIcon("fmt_strike", ThemeColor::DefaultColor, size), tr("Strikethrough"));
    actStrikethrough->setCheckable(true);
    actStrikethrough->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    connect(actStrikethrough, &QAction::toggled, m_editor, &GuiTextEditor::toggleStrikeOut);

    actSuperscript = this->addAction(m_theme->icons()->getIcon("fmt_superscript", ThemeColor::DefaultColor, size), tr("Superscript"));
    actSuperscript->setCheckable(true);
    actSuperscript->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Equal));
    connect(actSuperscript, &QAction::toggled, m_editor, &GuiTextEditor::toggleSuperscript);

    actSubscript = this->addAction(m_theme->icons()->getIcon("fmt_subscript", ThemeColor::DefaultColor, size), tr("Subscript"));
    actSubscript->setCheckable(true);
    actSubscript->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Equal));
    connect(actSubscript, &QAction::toggled, m_editor, &GuiTextEditor::toggleSubscript);

    connect(m_editor, &QTextEdit::currentCharFormatChanged, this, &GuiEditorToolBar::updateFormatButtons);
}

GuiEditorToolBar::~GuiEditorToolBar()
{
    qDebug() << "Destructor: GuiEditorToolBar";
}

// Private Slots
// =============

void GuiEditorToolBar::updateFormatButtons(const QTextCharFormat &format)
{
    actBold->blockSignals(true);
    actBold->setChecked(format.fontWeight() == QFont::Bold);
    actBold->blockSignals(false);

    actItalic->blockSignals(true);
    actItalic->setChecked(format.fontItalic());
    actItalic->blockSignals(false);

    actUnderline->blockSignals(true);
    actUnderline->setChecked(format.fontUnderline());
    actUnderline->blockSignals(false);

    actStrikethrough->blockSignals(true);
    actStrikethrough->setChecked(format.fontStrikeOut());
    actStrikethrough->blockSignals(false);

    actSuperscript->blockSignals(true);
    actSuperscript->setChecked(format.verticalAlignment() == QTextCharFormat::AlignSuperScript);
    actSuperscript->blockSignals(false);

    actSubscript->blockSignals(true);
    actSubscript->setChecked(format.verticalAlignment() == QTextCharFormat::AlignSubScript);
    actSubscript->blockSignals(false);
}

} // namespace Collett
