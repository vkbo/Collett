/*
** Collett - Text Counting Tools
** =============================
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

#include "counting.h"

#include <QChar>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextDocument>

namespace Collett {

// Snapshot
// ========

/**! @brief Snapshot every block of a document for counting.
 *
 * Must be called on the thread that owns the document.
 */
CountBlockList TextCounter::snapshot(const QTextDocument *doc)
{
    CountBlockList blocks;
    if (doc == nullptr) {
        return blocks;
    }
    blocks.reserve(doc->blockCount());
    for (QTextBlock block = doc->firstBlock(); block.isValid(); block = block.next()) {
        blocks.append(snapshotBlock(block));
    }
    return blocks;
}

/**! @brief Snapshot a single block for counting.
 *
 * Heading blocks are those with a heading level set, and comment blocks are
 * those with the comment block type property.
 */
CountBlock TextCounter::snapshotBlock(const QTextBlock &block)
{
    QTextBlockFormat format = block.blockFormat();
    CountBlock item;
    item.text = block.text();
    item.heading = format.headingLevel() > 0;
    item.comment = format.intProperty(BlockTypeProperty) == CommentBlock;
    return item;
}

// Counters
// ========

/**! @brief The standard count of characters, words and paragraphs.
 *
 * Comment blocks are skipped. Headings count towards the characters and
 * words, but are not paragraphs. Every other non-empty block is a paragraph.
 * Trailing whitespace on a block is not counted as characters.
 */
TextCounts TextCounter::standardCount(const CountBlockList &blocks)
{
    TextCounts counts;
    for (const CountBlock &block : blocks) {
        if (block.comment) {
            continue;
        }
        qint32 characters = countCharacters(block.text);
        if (characters == 0) {
            continue;
        }
        counts.characters += characters;
        counts.words += countWords(block.text);
        if (!block.heading) {
            counts.paragraphs++;
        }
    }
    return counts;
}

/**! @brief The standard count of a plain text, one paragraph per line.
 */
TextCounts TextCounter::standardCount(const QString &text)
{
    CountBlockList blocks;
    const QStringList lines = text.split(u'\n');
    blocks.reserve(lines.size());
    for (const QString &line : lines) {
        blocks.append({line, false, false});
    }
    return standardCount(blocks);
}

// Helpers
// =======

/**! @brief Count the words in a text.
 *
 * A word is a run of characters between word separators.
 */
qint32 TextCounter::countWords(const QString &text)
{
    qint32 words = 0;
    bool inWord = false;
    for (const QChar &c : text) {
        if (isWordSeparator(c)) {
            inWord = false;
        } else if (!inWord) {
            inWord = true;
            words++;
        }
    }
    return words;
}

/**! @brief Count the characters in a text, ignoring trailing whitespace.
 */
qint32 TextCounter::countCharacters(const QString &text)
{
    qsizetype length = text.size();
    while (length > 0 && text.at(length - 1).isSpace()) {
        length--;
    }
    return qint32(length);
}

/**! @brief Check if a character separates words.
 *
 * Whitespace, including the line separator used for soft line breaks, and
 * en and em dashes separate words.
 */
bool TextCounter::isWordSeparator(const QChar &c)
{
    return c.isSpace() || c == u'–' || c == u'—';
}

} // namespace Collett
