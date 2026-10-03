/*
** Collett - Nuspell Spell Check Engine
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

#pragma once

#include "collett.h"
#include "spellengine.h"

#include <QList>
#include <QString>
#include <QStringList>

#include <memory>

namespace Collett {

/**! @brief Spell check engine backed by the Nuspell library.
 *
 * Nuspell reads the same .aff/.dic dictionary pairs as Hunspell. The library
 * types are kept out of this header so that only this translation unit needs
 * the Nuspell headers.
 */
class NuspellEngine : public SpellEngine
{
public:
    NuspellEngine();
    ~NuspellEngine() override;

    // Methods
    bool loadLanguage(const QString &tag, const QStringList &extraDirs = QStringList());
    bool loadDictionary(const QString &affPath, const QString &tag = QString());
    void unload();

    // Static Methods
    static QList<DictionaryInfo> availableDictionaries(const QStringList &extraDirs = QStringList());

    // SpellEngine Interface
    bool isLoaded() const override { return m_isLoaded; };
    QString language() const override { return m_language; };
    QString provider() const override { return QStringLiteral("Nuspell"); };
    bool spell(const QString &word) const override;
    QStringList suggest(const QString &word) const override;

private:
    struct Private;
    std::unique_ptr<Private> d;

    bool m_isLoaded = false;
    QString m_language;
};

} // namespace Collett
