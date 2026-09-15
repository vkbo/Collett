/*
** Collett - Text Block Data Class
** ===============================
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

#include "textblock.h"

#include <QChar>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>

using namespace Qt::Literals::StringLiterals;

namespace Collett {

// Same word split rule as novelWriter: a run of characters between word
// boundaries that are not whitespace or one of the separators that should
// split words, like hyphens and dashes.
static const QRegularExpression RX_WORDS(uR"(\b[^\s\-\+\/–—\[\]:]+\b)"_s, QRegularExpression::UseUnicodePropertiesOption);
static const QRegularExpression RX_MULTI_SPACE(u" {2,}"_s);
static const QRegularExpression RX_TRAIL_SPACE(u"[ \\t]+$"_s);

/**! @brief Check if all characters of a word are numeric.
 */
static bool isNumeric(const QString &word)
{
    for (const QChar &c : word) {
        if (!c.isNumber()) {
            return false;
        }
    }
    return !word.isEmpty();
}

/**! @brief Check if a word is all upper case, like an acronym.
 *
 * True when the word has at least one letter and no lower case letters.
 */
static bool isUpper(const QString &word)
{
    bool hasLetter = false;
    for (const QChar &c : word) {
        if (c.isLetter()) {
            if (c.isLower()) {
                return false;
            }
            hasLetter = true;
        }
    }
    return hasLetter;
}

// Public Methods
// ==============

/**! @brief Store a new snapshot of the block text.
 *
 * Cached errors are cleared and left for the background check to recompute,
 * and the revision is bumped so results from a check of the old text are
 * recognised as stale.
 */
void TextBlockData::processText(const QString &text)
{
    m_text = text;
    m_revision++;
    m_spellErrors.clear();
    m_formatErrors.clear();
}

/**! @brief Clear the snapshot and all cached errors.
 */
void TextBlockData::clear()
{
    m_text.clear();
    m_revision++;
    m_spellErrors.clear();
    m_formatErrors.clear();
}

// Text Check Functions
// ====================

/**! @brief Spell check a piece of text and return the misspelled words.
 *
 * Numbers and all upper case words are skipped, as novelWriter does. The
 * modifier letter apostrophe is checked as a plain apostrophe. A null spell
 * checker flags nothing.
 */
TextCheckList spellCheckText(const QString &text, SpellChecker *spell)
{
    TextCheckList errors;
    if (spell == nullptr) {
        return errors;
    }
    QRegularExpressionMatchIterator it = RX_WORDS.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString word = match.captured(0);
        if (word.isEmpty() || isNumeric(word) || isUpper(word)) {
            continue;
        }
        if (!spell->checkWord(word.replace(u'ʼ', u'\''))) {
            errors.append({int(match.capturedStart(0)), int(match.capturedEnd(0)), match.captured(0)});
        }
    }
    return errors;
}

/**! @brief Check a piece of text for redundant spaces.
 *
 * Runs of two or more spaces are flagged as "multi", and trailing whitespace
 * as "trail".
 */
TextCheckList formatCheckText(const QString &text)
{
    TextCheckList errors;
    QRegularExpressionMatchIterator it = RX_MULTI_SPACE.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        errors.append({int(match.capturedStart(0)), int(match.capturedEnd(0)), u"multi"_s});
    }
    QRegularExpressionMatch trail = RX_TRAIL_SPACE.match(text);
    if (trail.hasMatch()) {
        errors.append({int(trail.capturedStart(0)), int(trail.capturedEnd(0)), u"trail"_s});
    }
    return errors;
}

} // namespace Collett
