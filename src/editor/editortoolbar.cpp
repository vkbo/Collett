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
#include <QActionGroup>
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

    this->addSeparator();

    QActionGroup *alignGroup = new QActionGroup(this);

    actAlignLeft = this->addAction(m_theme->icons()->getIcon("fmt_align_left", ThemeColor::DefaultColor, size), tr("Align Left"));
    actAlignLeft->setCheckable(true);
    actAlignLeft->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    alignGroup->addAction(actAlignLeft);
    connect(actAlignLeft, &QAction::triggered, m_editor, &GuiTextEditor::alignLeft);

    actAlignCenter = this->addAction(m_theme->icons()->getIcon("fmt_align_center", ThemeColor::DefaultColor, size), tr("Align Center"));
    actAlignCenter->setCheckable(true);
    actAlignCenter->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
    alignGroup->addAction(actAlignCenter);
    connect(actAlignCenter, &QAction::triggered, m_editor, &GuiTextEditor::alignCenter);

    actAlignRight = this->addAction(m_theme->icons()->getIcon("fmt_align_right", ThemeColor::DefaultColor, size), tr("Align Right"));
    actAlignRight->setCheckable(true);
    actAlignRight->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    alignGroup->addAction(actAlignRight);
    connect(actAlignRight, &QAction::triggered, m_editor, &GuiTextEditor::alignRight);

    actAlignJustify = this->addAction(m_theme->icons()->getIcon("fmt_align_justify", ThemeColor::DefaultColor, size), tr("Justify"));
    actAlignJustify->setCheckable(true);
    actAlignJustify->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_J));
    alignGroup->addAction(actAlignJustify);
    connect(actAlignJustify, &QAction::triggered, m_editor, &GuiTextEditor::alignJustify);

    this->addSeparator();

    actIndent = this->addAction(m_theme->icons()->getIcon("fmt_indent", ThemeColor::DefaultColor, size), tr("Increase Indent"));
    actIndent->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_M));
    connect(actIndent, &QAction::triggered, m_editor, &GuiTextEditor::blockIndent);

    actOutdent = this->addAction(m_theme->icons()->getIcon("fmt_outdent", ThemeColor::DefaultColor, size), tr("Decrease Indent"));
    actOutdent->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_M));
    connect(actOutdent, &QAction::triggered, m_editor, &GuiTextEditor::blockOutdent);

    connect(m_editor, &QTextEdit::currentCharFormatChanged, this, &GuiEditorToolBar::updateFormatButtons);
    connect(m_editor, &QTextEdit::cursorPositionChanged, this, &GuiEditorToolBar::updateAlignButtons);
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

void GuiEditorToolBar::updateAlignButtons()
{
    switch (m_editor->alignment()) {
    case Qt::AlignHCenter:
        actAlignCenter->setChecked(true);
        break;
    case Qt::AlignRight:
        actAlignRight->setChecked(true);
        break;
    case Qt::AlignJustify:
        actAlignJustify->setChecked(true);
        break;
    default:
        actAlignLeft->setChecked(true);
        break;
    }
}

} // namespace Collett
