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
#include "mtoolbutton.h"
#include "texteditor.h"

#include <QAction>
#include <QActionGroup>
#include <QKeySequence>
#include <QTextCharFormat>
#include <QTextEdit>
#include <QToolBar>

namespace Collett {

// Constructor/Destructor
// ======================

GuiEditorToolBar::GuiEditorToolBar(GuiTextEditor *editor, QWidget *parent)
    : QToolBar(parent), m_editor(editor)
{
    actBold = new QAction(tr("Bold"), this);
    this->addToolButton(actBold, "fmt_bold", ThemeColor::ToolColor);
    actBold->setCheckable(true);
    actBold->setShortcut(QKeySequence::Bold);
    connect(actBold, &QAction::toggled, m_editor, &GuiTextEditor::toggleBold);

    actItalic = new QAction(tr("Italic"), this);
    this->addToolButton(actItalic, "fmt_italic", ThemeColor::ToolColor);
    actItalic->setCheckable(true);
    actItalic->setShortcut(QKeySequence::Italic);
    connect(actItalic, &QAction::toggled, m_editor, &GuiTextEditor::toggleItalic);

    actUnderline = new QAction(tr("Underline"), this);
    this->addToolButton(actUnderline, "fmt_underline", ThemeColor::ToolColor);
    actUnderline->setCheckable(true);
    actUnderline->setShortcut(QKeySequence::Underline);
    connect(actUnderline, &QAction::toggled, m_editor, &GuiTextEditor::toggleUnderline);

    actStrikethrough = new QAction(tr("Strikethrough"), this);
    this->addToolButton(actStrikethrough, "fmt_strike", ThemeColor::ToolColor);
    actStrikethrough->setCheckable(true);
    actStrikethrough->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    connect(actStrikethrough, &QAction::toggled, m_editor, &GuiTextEditor::toggleStrikeOut);

    actSuperscript = new QAction(tr("Superscript"), this);
    this->addToolButton(actSuperscript, "fmt_superscript", ThemeColor::ToolColor);
    actSuperscript->setCheckable(true);
    actSuperscript->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Equal));
    connect(actSuperscript, &QAction::toggled, m_editor, &GuiTextEditor::toggleSuperscript);

    actSubscript = new QAction(tr("Subscript"), this);
    this->addToolButton(actSubscript, "fmt_subscript", ThemeColor::ToolColor);
    actSubscript->setCheckable(true);
    actSubscript->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Equal));
    connect(actSubscript, &QAction::toggled, m_editor, &GuiTextEditor::toggleSubscript);

    this->addSeparator();

    QActionGroup *alignGroup = new QActionGroup(this);

    actAlignLeft = new QAction(tr("Align Left"), this);
    this->addToolButton(actAlignLeft, "fmt_align_left", ThemeColor::ToolColor);
    actAlignLeft->setCheckable(true);
    actAlignLeft->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    alignGroup->addAction(actAlignLeft);
    connect(actAlignLeft, &QAction::triggered, m_editor, &GuiTextEditor::alignLeft);

    actAlignCenter = new QAction(tr("Align Center"), this);
    this->addToolButton(actAlignCenter, "fmt_align_center", ThemeColor::ToolColor);
    actAlignCenter->setCheckable(true);
    actAlignCenter->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
    alignGroup->addAction(actAlignCenter);
    connect(actAlignCenter, &QAction::triggered, m_editor, &GuiTextEditor::alignCenter);

    actAlignRight = new QAction(tr("Align Right"), this);
    this->addToolButton(actAlignRight, "fmt_align_right", ThemeColor::ToolColor);
    actAlignRight->setCheckable(true);
    actAlignRight->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    alignGroup->addAction(actAlignRight);
    connect(actAlignRight, &QAction::triggered, m_editor, &GuiTextEditor::alignRight);

    actAlignJustify = new QAction(tr("Justify"), this);
    this->addToolButton(actAlignJustify, "fmt_align_justify", ThemeColor::ToolColor);
    actAlignJustify->setCheckable(true);
    actAlignJustify->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_J));
    alignGroup->addAction(actAlignJustify);
    connect(actAlignJustify, &QAction::triggered, m_editor, &GuiTextEditor::alignJustify);

    this->addSeparator();

    actIndent = new QAction(tr("Increase Indent"), this);
    this->addToolButton(actIndent, "fmt_indent", ThemeColor::ToolColor);
    actIndent->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_M));
    connect(actIndent, &QAction::triggered, m_editor, &GuiTextEditor::blockIndent);

    actOutdent = new QAction(tr("Decrease Indent"), this);
    this->addToolButton(actOutdent, "fmt_outdent", ThemeColor::ToolColor);
    actOutdent->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_M));
    connect(actOutdent, &QAction::triggered, m_editor, &GuiTextEditor::blockOutdent);

    connect(m_editor, &QTextEdit::currentCharFormatChanged, this, &GuiEditorToolBar::updateFormatButtons);
    connect(m_editor, &QTextEdit::cursorPositionChanged, this, &GuiEditorToolBar::updateAlignButtons);
}

GuiEditorToolBar::~GuiEditorToolBar()
{
    qDebug() << "Destructor: GuiEditorToolBar";
}

// Private Helpers
// ===============

/**! @brief Add a tool button for an action with an icon from the theme.
 */
void GuiEditorToolBar::addToolButton(QAction *action, const QString &icon, ThemeColor color)
{
    MToolButton *button = new MToolButton(this);
    button->setDefaultAction(action);
    button->setThemeIcon(icon, color);
    this->addWidget(button);
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
