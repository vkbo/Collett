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

#include <QTextDocument>

using namespace Qt::Literals::StringLiterals;

namespace Collett {

// Constructor/Destructor
// ======================

Highlighter::Highlighter(QObject *parent) : QSyntaxHighlighter(parent)
{
    m_fmtSpellError.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    m_fmtSpellError.setUnderlineColor(Qt::red);
    m_fmtFormatError.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    m_fmtFormatError.setUnderlineColor(QColor(255, 165, 0));
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

/**! @brief Set the colour of the line under redundant spaces.
 */
void Highlighter::setFormatErrorColor(const QColor &color)
{
    if (m_fmtFormatError.underlineColor() == color) return;
    m_fmtFormatError.setUnderlineColor(color);
    this->rehighlight();
}

/**! @brief Set whether runs of spaces and trailing spaces are underlined.
 */
void Highlighter::setCheckFormat(bool enabled)
{
    if (m_checkFormat == enabled) return;
    m_checkFormat = enabled;
    this->rehighlight();
}

/**! @brief Set the position of the editor's cursor in the document.
 *
 * Trailing spaces with the cursor in them, or right after them, are not
 * underlined, as they are most likely still being typed. The blocks where
 * this changes are checked again.
 */
void Highlighter::setCursorPosition(int position)
{
    if (m_cursor == position) return;
    QTextDocument *doc = this->document();
    if (!doc || !m_checkFormat) {
        m_cursor = position;
        return;
    }

    const QTextBlock oldBlock = doc->findBlock(m_cursor);
    const QTextBlock newBlock = doc->findBlock(position);
    const bool oldBefore = trailUnderCursor(oldBlock, m_cursor);
    const bool newBefore = trailUnderCursor(newBlock, m_cursor);
    m_cursor = position;
    if (oldBlock != newBlock && oldBefore) this->rehighlightBlock(oldBlock);
    if (newBefore != trailUnderCursor(newBlock, position)) this->rehighlightBlock(newBlock);
}

// Protected Methods
// =================

/**! @brief Underline the redundant spaces and misspelled words of a block.
 */
void Highlighter::highlightBlock(const QString &text)
{
    if (m_checkFormat) {
        const int cursor = m_cursor - this->currentBlock().position();
        const TextCheckList formatErrors = formatCheckText(text);
        for (const TextCheck &error : formatErrors) {
            if (error.text == "trail"_L1 && error.start < cursor && cursor <= error.end) continue;
            this->setFormat(error.start, error.end - error.start, m_fmtFormatError);
        }
    }

    if (!m_spell) return;
    const TextCheckList spellErrors = spellCheckText(text, m_spell);
    for (const TextCheck &error : spellErrors) {
        this->setFormat(error.start, error.end - error.start, m_fmtSpellError);
    }
}

// Private Methods
// ===============

/**! @brief Whether a position is in, or right after, the trailing spaces of
 * a block.
 */
bool Highlighter::trailUnderCursor(const QTextBlock &block, int position) const
{
    if (!block.isValid()) return false;
    const QString text = block.text();
    qsizetype start = text.size();
    while (start > 0 && (text.at(start - 1) == u' ' || text.at(start - 1) == u'\t')) {
        start--;
    }
    const int cursor = position - block.position();
    return start < text.size() && start < cursor && cursor <= text.size();
}

} // namespace Collett
