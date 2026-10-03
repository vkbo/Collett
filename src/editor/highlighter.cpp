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

#include <QTextBlock>
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

/**! @brief Stop recording edits before the base class lets go of the
 * document, as that changes it.
 */
Highlighter::~Highlighter()
{
    if (QTextDocument *doc = this->document()) doc->disconnect(this);
}

// Methods
// =======

/**! @brief Set the document to mark.
 *
 * Edits to the document are recorded before the highlighter checks the
 * changed text, so the check knows which marks are being typed.
 */
void Highlighter::setDocument(QTextDocument *doc)
{
    if (QTextDocument *old = this->document()) old->disconnect(this);
    if (doc) connect(doc, &QTextDocument::contentsChange, this, &Highlighter::recordEdit);
    QSyntaxHighlighter::setDocument(doc);
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
        connect(m_spell, &SpellChecker::languageChanged, this, &Highlighter::recheck);
        connect(m_spell, &SpellChecker::userDictionaryChanged, this, &Highlighter::recheck);
    }
    this->recheck();
}

void Highlighter::setErrorColor(const QColor &color)
{
    if (m_fmtSpellError.underlineColor() == color) return;
    m_fmtSpellError.setUnderlineColor(color);
    this->recheck();
}

/**! @brief Set the colour of the line under redundant spaces.
 */
void Highlighter::setFormatErrorColor(const QColor &color)
{
    if (m_fmtFormatError.underlineColor() == color) return;
    m_fmtFormatError.setUnderlineColor(color);
    this->recheck();
}

/**! @brief Set whether runs of spaces and trailing spaces are underlined.
 */
void Highlighter::setCheckFormat(bool enabled)
{
    if (m_checkFormat == enabled) return;
    m_checkFormat = enabled;
    this->recheck();
}

/**! @brief Set the position of the editor's cursor in the document.
 *
 * A mark hidden while it was being typed is shown again once the cursor is
 * no longer in it, or right after it. Moving the cursor into a mark does not
 * hide it, so a word can be found and corrected.
 */
void Highlighter::setCursorPosition(int position)
{
    if (m_cursor == position) return;
    m_cursor = position;
    showLeftMarks();
}

// Protected Methods
// =================

/**! @brief Underline the redundant spaces and misspelled words of a block.
 *
 * After an edit, a misspelled word or trailing space touching the edited
 * text is left out, as it is most likely still being typed, and remembered
 * as hidden.
 */
void Highlighter::highlightBlock(const QString &text)
{
    const int position = this->currentBlock().position();
    const int editStart = m_editStart - position;
    const int editEnd = m_editEnd - position;
    m_hidden.removeIf([position, &text](const QPair<int, int> &range) {
        return range.first >= position && range.first <= position + text.size();
    });

    auto mark = [this, position, editStart, editEnd](const TextCheck &error, const QTextCharFormat &format) {
        if (!m_showAll && m_cursor >= 0 && m_editStart >= 0 && error.start <= editEnd && editStart <= error.end) {
            m_hidden.append({position + error.start, position + error.end});
        } else {
            this->setFormat(error.start, error.end - error.start, format);
        }
    };

    if (m_checkFormat) {
        const TextCheckList formatErrors = formatCheckText(text);
        for (const TextCheck &error : formatErrors) {
            if (error.text == "trail"_L1) {
                mark(error, m_fmtFormatError);
            } else {
                this->setFormat(error.start, error.end - error.start, m_fmtFormatError);
            }
        }
    }

    if (!m_spell) return;
    const TextCheckList spellErrors = spellCheckText(text, m_spell);
    for (const TextCheck &error : spellErrors) {
        mark(error, m_fmtSpellError);
    }
}

// Private Methods
// ===============

/**! @brief Record the text changed by an edit.
 *
 * Changes that only touch the format, including those made by the
 * highlighter itself, are not edits of the text.
 */
void Highlighter::recordEdit(int position, int removed, int added)
{
    if (removed == added) return;
    m_editStart = position;
    m_editEnd = position + added;
    QMetaObject::invokeMethod(this, &Highlighter::showLeftMarks, Qt::QueuedConnection);
}

/**! @brief Show the hidden marks that the cursor is no longer in, or right
 * after. This also covers edits that do not move the cursor.
 */
void Highlighter::showLeftMarks()
{
    m_editStart = -1;
    m_editEnd = -1;
    QTextDocument *doc = this->document();
    if (!doc) return;

    QList<int> left;
    for (const auto &[start, end] : std::as_const(m_hidden)) {
        if (!(start < m_cursor && m_cursor <= end)) left.append(start);
    }
    for (int start : std::as_const(left)) {
        recheckBlock(doc->findBlock(start));
    }
}

/**! @brief Check the whole document again, showing all marks.
 */
void Highlighter::recheck()
{
    m_showAll = true;
    this->rehighlight();
    m_showAll = false;
}

/**! @brief Check a block again, showing all its marks.
 */
void Highlighter::recheckBlock(const QTextBlock &block)
{
    m_showAll = true;
    this->rehighlightBlock(block);
    m_showAll = false;
}

} // namespace Collett
