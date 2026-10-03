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
#include <QPointer>
#include <QString>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

namespace Collett {

/**! @brief Marks misspelled words in a document.
 *
 * The marks are layout overlays, so they are never stored with the document,
 * and do not mark it as modified or add to its undo history.
 */
class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit Highlighter(QObject *parent = nullptr);

    // Setters
    void setSpellChecker(SpellChecker *spell);
    void setErrorColor(const QColor &color);

protected:
    void highlightBlock(const QString &text) override;

private:
    QPointer<SpellChecker> m_spell;
    QTextCharFormat m_fmtSpellError;
};

} // namespace Collett
