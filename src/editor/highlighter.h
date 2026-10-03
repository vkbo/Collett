/*
** Collett - Document Highlighter Class
** ====================================
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
#include "spellchecker.h"

#include <QColor>
#include <QList>
#include <QPair>
#include <QPointer>
#include <QString>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QTextCharFormat>

namespace Collett {

/**! @brief Marks misspelled words and redundant spaces in a document.
 *
 * The marks are layout overlays, so they are never stored with the document,
 * and do not mark it as modified or add to its undo history.
 */
class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit Highlighter(QObject *parent = nullptr);
    ~Highlighter();

    void setDocument(QTextDocument *doc);

    // Setters
    void setSpellChecker(SpellChecker *spell);
    void setErrorColor(const QColor &color);
    void setFormatErrorColor(const QColor &color);
    void setCheckFormat(bool enabled);
    void setCursorPosition(int position);

protected:
    void highlightBlock(const QString &text) override;

private:
    QPointer<SpellChecker> m_spell;
    QTextCharFormat m_fmtSpellError;
    QTextCharFormat m_fmtFormatError;
    bool m_checkFormat = false;
    int m_cursor = -1;
    bool m_showAll = false;

    // The range of text changed by the last edit, as document positions
    int m_editStart = -1;
    int m_editEnd = -1;

    // Marks left out because the cursor is in them, as document ranges
    QList<QPair<int, int>> m_hidden;

    void recheck();
    void recheckBlock(const QTextBlock &block);
    void recordEdit(int position, int removed, int added);
    void showLeftMarks();
};

} // namespace Collett
