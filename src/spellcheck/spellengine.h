/*
** Collett - Spell Check Engine Interface
** ======================================
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
#include <QStringList>

namespace Collett {

/**! @brief A dictionary available to a spell check engine.
 *
 * The tag is the language code of the dictionary, like "en_GB", and the path
 * points to the file the engine needs to load it.
 */
struct DictionaryInfo
{
    QString tag;
    QString path;
};

/**! @brief Abstract interface for a spell check engine.
 *
 * The engine only knows how to check single words against a loaded language
 * dictionary. User words, caching and project awareness are handled by the
 * SpellChecker class on top of it, so an engine can be swapped without
 * touching the rest of the application.
 */
class SpellEngine
{
public:
    virtual ~SpellEngine() = default;

    virtual bool isLoaded() const = 0;
    virtual QString language() const = 0;
    virtual QString provider() const = 0;
    virtual bool spell(const QString &word) const = 0;
    virtual QStringList suggest(const QString &word) const = 0;
};

/**! @brief Fallback engine used when no dictionary is loaded.
 *
 * Accepts every word and never suggests anything, so lookups keep working
 * when the requested language is unavailable.
 */
class NullSpellEngine : public SpellEngine
{
public:
    bool isLoaded() const override { return false; };
    QString language() const override { return QString(); };
    QString provider() const override { return QString(); };
    bool spell(const QString &word) const override;
    QStringList suggest(const QString &word) const override;
};

} // namespace Collett
