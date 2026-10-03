/*
** Collett - Text Checks
** =====================
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

#include <QList>
#include <QString>

namespace Collett {

/**! @brief A range flagged by a text check, relative to the block start.
 *
 * For spell errors the text is the misspelled word. For format errors it is
 * the kind of error, like "multi" or "trail".
 */
struct TextCheck
{
    int start = 0;
    int end = 0;
    QString text;

    bool operator==(const TextCheck &other) const
    {
        return start == other.start && end == other.end && text == other.text;
    }
    bool operator!=(const TextCheck &other) const { return !(*this == other); }
};
using TextCheckList = QList<TextCheck>;

// Text Check Functions
// These only work on plain strings, so they are safe to call from a worker
// thread.
TextCheckList spellCheckText(const QString &text, SpellChecker *spell);
TextCheckList formatCheckText(const QString &text);

} // namespace Collett
