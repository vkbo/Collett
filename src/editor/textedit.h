/*
** Collett – GUI Text Editor Class
** ===============================
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

#ifndef GUI_TEXTEDIT_H
#define GUI_TEXTEDIT_H

#include "collett.h"
#include "settings.h"
#include "data.h"

#include <QObject>
#include <QWidget>
#include <QTextEdit>
#include <QJsonArray>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextBlockFormat>

namespace Collett {

class GuiTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    enum BlockFormat {
        Paragraph, Header, BlockQuote
    };

    GuiTextEdit(QWidget *parent=nullptr);
    ~GuiTextEdit() {};

    // Setters

    void setModified(bool state);

    // Getters

    bool isModified() const;

    // Methods

    QJsonArray toJsonContent();
    void setJsonContent(const QJsonArray &json);

private:
    CollettSettings::TextFormat m_format;

    int m_currentBlockNo = -1;

    void initDocument(QTextDocument *doc);

signals:
    void currentBlockChanged(const QTextBlock &block);

public slots:
    void toggleBoldFormat();
    void toggleItalicFormat();
    void toggleUnderlineFormat();
    void toggleStrikeOutFormat();
    void toggleSuperScriptFormat();
    void toggleSubScriptFormat();
    void toggleSegmentFormat();
    void toggleFirstLineIndent();
    void increaseBlockIndent();
    void decreaseBlockIndent();
    void applyBlockAlignment(Qt::Alignment align);
    void applyBlockFormat(BlockFormat format, int hLevel);

private slots:
    void processCursorPositionChanged();

};
} // namespace Collett

#endif // GUI_TEXTEDIT_H
