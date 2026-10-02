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

#pragma once

#include "collett.h"

#include <QList>
#include <QMetaType>
#include <QString>
#include <QTextDocument>

namespace Collett {

/**! @brief The result of counting a text.
 */
struct TextCounts
{
    qint32 characters = 0;
    qint32 words = 0;
    qint32 paragraphs = 0;

    bool operator==(const TextCounts &other) const
    {
        return characters == other.characters && words == other.words && paragraphs == other.paragraphs;
    }
    bool operator!=(const TextCounts &other) const { return !(*this == other); }
    TextCounts &operator+=(const TextCounts &other)
    {
        characters += other.characters;
        words += other.words;
        paragraphs += other.paragraphs;
        return *this;
    }
};

/**! @brief A block snapshot for counting, detached from the document.
 *
 * The snapshot only holds what the counters need to know about a block, so
 * it can be counted on a worker thread without touching the document.
 */
struct CountBlock
{
    QString text;
    bool heading = false;
    bool comment = false;
};
using CountBlockList = QList<CountBlock>;

/**! @brief Text counting tool.
 *
 * Counts are made from a list of block snapshots, taken from a document on
 * the GUI thread with snapshot(). The counters themselves only work on the
 * snapshot, so they are safe to run on a worker thread. Words are separated
 * by whitespace, and by en and em dashes, but not by hyphens.
 */
class TextCounter
{
public:
    // Snapshot
    static CountBlockList snapshot(const QTextDocument *doc);
    static CountBlock snapshotBlock(const QTextBlock &block);

    // Counters
    static TextCounts standardCount(const CountBlockList &blocks);
    static TextCounts standardCount(const QString &text);

    // Helpers
    static qint32 countWords(const QString &text);
    static qint32 countCharacters(const QString &text);
    static bool isWordSeparator(const QChar &c);
};

} // namespace Collett

Q_DECLARE_METATYPE(Collett::TextCounts)
