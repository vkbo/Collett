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

#pragma once

#include "collett.h"
#include "document.h"

#include <QKeyEvent>
#include <QTextCharFormat>
#include <QTextEdit>

namespace Collett {

class GuiTextEditor : public QTextEdit
{
    Q_OBJECT

public:
    explicit GuiTextEditor(QWidget *parent = nullptr);
    ~GuiTextEditor();

    // Methods
    void openDocument(Document *doc);

protected:
    void keyPressEvent(QKeyEvent *event) override;

public slots:
    void toggleBold(bool bold);
    void toggleItalic(bool italic);
    void toggleUnderline(bool underline);
    void toggleStrikeOut(bool strikeOut);
    void toggleSuperscript(bool superscript);
    void toggleSubscript(bool subscript);
    void alignLeft();
    void alignCenter();
    void alignRight();
    void alignJustify();
    void blockIndent();
    void blockOutdent();

private:
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
};
} // namespace Collett
