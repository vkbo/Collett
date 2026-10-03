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

#include "highlighter.h"
#include "textcheck.h"

namespace Collett {

// Constructor/Destructor
// ======================

Highlighter::Highlighter(QObject *parent) : QSyntaxHighlighter(parent)
{
    m_fmtSpellError.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    m_fmtSpellError.setUnderlineColor(Qt::red);
}

// Setters
// =======

/**! @brief Set the spell checker, and check again whenever its language or
 * user dictionary changes.
 */
void Highlighter::setSpellChecker(SpellChecker *spell)
{
    if (m_spell == spell) return;
    if (m_spell) m_spell->disconnect(this);
    m_spell = spell;
    if (m_spell) {
        connect(m_spell, &SpellChecker::languageChanged, this, &Highlighter::rehighlight);
        connect(m_spell, &SpellChecker::userDictionaryChanged, this, &Highlighter::rehighlight);
    }
    this->rehighlight();
}

void Highlighter::setErrorColor(const QColor &color)
{
    if (m_fmtSpellError.underlineColor() == color) return;
    m_fmtSpellError.setUnderlineColor(color);
    this->rehighlight();
}

// Protected Methods
// =================

/**! @brief Underline the misspelled words of a block.
 */
void Highlighter::highlightBlock(const QString &text)
{
    if (!m_spell) return;
    const TextCheckList errors = spellCheckText(text, m_spell);
    for (const TextCheck &error : errors) {
        this->setFormat(error.start, error.end - error.start, m_fmtSpellError);
    }
}

} // namespace Collett
