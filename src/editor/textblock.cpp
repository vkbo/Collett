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

namespace Collett {

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

} // namespace Collett
