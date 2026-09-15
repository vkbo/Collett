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

#pragma once

#include "collett.h"

#include <QList>
#include <QString>
#include <QTextBlockUserData>

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

/**! @brief Custom data attached to each text block by the highlighter.
 *
 * Holds a snapshot of the block's text for the background text checks, and
 * the errors those checks found. The revision counter goes up every time the
 * text is snapshotted, so a check result can be discarded if the block was
 * edited while the check was running.
 */
class TextBlockData : public QTextBlockUserData
{
public:
    TextBlockData() = default;
    ~TextBlockData() override = default;

    // Methods
    void processText(const QString &text);
    void clear();

    // Setters
    void setSpellErrors(const TextCheckList &errors) { m_spellErrors = errors; };
    void setFormatErrors(const TextCheckList &errors) { m_formatErrors = errors; };

    // Getters
    QString text() const { return m_text; };
    int revision() const { return m_revision; };
    TextCheckList spellErrors() const { return m_spellErrors; };
    TextCheckList formatErrors() const { return m_formatErrors; };

private:
    QString m_text;
    int m_revision = 0;
    TextCheckList m_spellErrors;
    TextCheckList m_formatErrors;
};

} // namespace Collett
