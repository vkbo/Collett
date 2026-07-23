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

#pragma once

#include "collett.h"
#include "texteditor.h"
#include "theme.h"

#include <QAction>
#include <QTextCharFormat>
#include <QToolBar>
#include <QWidget>

namespace Collett {

class GuiEditorToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit GuiEditorToolBar(GuiTextEditor *editor, QWidget *parent = nullptr);
    ~GuiEditorToolBar();

private slots:
    void updateFormatButtons(const QTextCharFormat &format);

private:
    Theme *m_theme;
    GuiTextEditor *m_editor;

    QAction *actBold;
    QAction *actItalic;
    QAction *actUnderline;
    QAction *actStrikethrough;
    QAction *actSuperscript;
    QAction *actSubscript;
};
} // namespace Collett
