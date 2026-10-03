/*
** Collett - User Dictionary Class
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
#include "storage.h"

#include <QSet>
#include <QString>
#include <QStringList>

namespace Collett {

/**! @brief The user's own words for spell checking, saved per project.
 *
 * Entries are exact words, or stems marked with a trailing asterisk. A stem
 * entry like "Frodo*" accepts any word that starts with "Frodo", which covers
 * the inflected forms of names in languages like Finnish without listing each
 * one. Matching is case-sensitive, as it is for the dictionary itself.
 */
class UserDictionary
{
public:
    explicit UserDictionary(Storage *store = nullptr);

    // Methods
    bool contains(const QString &word) const;
    bool add(const QString &entry);
    bool remove(const QString &entry);
    void clear();
    bool load();
    bool save() const;

    // Static Methods
    static bool isStem(const QString &entry);

    // Setters
    void setStorage(Storage *store) { m_store = store; };

    // Getters
    QStringList entries() const;
    int count() const { return m_words.size() + m_stems.size(); };

private:
    Storage *m_store = nullptr;
    QSet<QString> m_words;
    QStringList m_stems;
};

} // namespace Collett
