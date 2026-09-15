/*
** Collett - Spell Checker Class
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
#include "spellengine.h"
#include "storage.h"
#include "userdictionary.h"

#include <QHash>
#include <QList>
#include <QMutex>
#include <QString>
#include <QStringList>

#include <memory>

namespace Collett {

/**! @brief The spell checking front end used by the rest of the application.
 *
 * Wraps a spell engine, the per-project user dictionary and a result cache
 * behind one API, so the editor never talks to the engine directly. A word
 * is accepted if either the engine or the user dictionary accepts it. When
 * the requested language cannot be loaded, a null engine that accepts every
 * word is used instead, so lookups always work.
 *
 * Word checks run on worker threads while the GUI thread may swap the
 * language or add words, so every access to the engine, the user dictionary
 * and the cache is guarded by a mutex. Signals are emitted outside the lock,
 * so a slot is free to call back into the checker.
 */
class SpellChecker : public QObject
{
    Q_OBJECT

public:
    struct Language
    {
        QString tag;
        QString name;
    };

    explicit SpellChecker(QObject *parent = nullptr);
    ~SpellChecker();

    // Methods
    bool checkWord(const QString &word);
    QStringList suggestWords(const QString &word) const;
    bool addWord(const QString &word, bool save = true);
    QList<Language> listDictionaries() const;

    // Static Methods
    static QString languageName(const QString &tag);

    // Setters
    void setStorage(Storage *store);
    void setLanguage(const QString &language);
    void setDictionaryPaths(const QStringList &paths);

    // Getters
    bool isLoaded() const { return m_engine->isLoaded(); };
    QString language() const { return m_engine->language(); };
    QString requestedLanguage() const { return m_requested; };
    QString provider() const { return m_engine->provider(); };
    QStringList dictionaryPaths() const { return m_dictPaths; };
    UserDictionary *userDictionary() { return &m_userDict; };

signals:
    void languageChanged(const QString &language);
    void userDictionaryChanged();

private:
    mutable QMutex m_mutex;
    std::unique_ptr<SpellEngine> m_engine;
    UserDictionary m_userDict;
    QHash<QString, bool> m_cache;
    QString m_requested;
    QStringList m_dictPaths;
};

} // namespace Collett
