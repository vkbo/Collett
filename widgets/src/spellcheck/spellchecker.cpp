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

#include "spellchecker.h"
#include "nuspellengine.h"

#include <QLocale>

#include <algorithm>

namespace Collett {

// The same words tend to be checked over and over, so results are cached.
// The cache is simply dropped when it grows past this size.
static const int MAX_CACHE_SIZE = 100000;

// Constructor/Destructor
// ======================

SpellChecker::SpellChecker(QObject *parent) : QObject(parent), m_engine(std::make_unique<NullSpellEngine>()) {}

SpellChecker::~SpellChecker()
{
    qDebug() << "Destructor: SpellChecker";
}

// Public Methods
// ==============

/**! @brief Check whether a word is spelled correctly.
 *
 * The engine is asked first, then the user dictionary. An empty word is
 * always correct. Results are cached per word.
 */
bool SpellChecker::checkWord(const QString &word)
{
    if (word.isEmpty()) {
        return true;
    }
    QMutexLocker locker(&m_mutex);
    auto cached = m_cache.constFind(word);
    if (cached != m_cache.constEnd()) {
        return cached.value();
    }
    bool result = m_engine->spell(word) || m_userDict.contains(word);
    if (m_cache.size() >= MAX_CACHE_SIZE) {
        m_cache.clear();
    }
    m_cache.insert(word, result);
    return result;
}

/**! @brief Ask the engine for spelling suggestions for a word.
 */
QStringList SpellChecker::suggestWords(const QString &word) const
{
    QMutexLocker locker(&m_mutex);
    return m_engine->suggest(word);
}

/**! @brief Add a word or stem to the project's user dictionary.
 *
 * A stem entry can turn previously rejected words into accepted ones, so the
 * whole cache is dropped for those. A plain word only needs its own cache
 * entry updated. Returns true if the entry was new.
 */
bool SpellChecker::addWord(const QString &word, bool save)
{
    QString entry = word.trimmed();
    {
        QMutexLocker locker(&m_mutex);
        if (!m_userDict.add(entry)) {
            return false;
        }
        if (UserDictionary::isStem(entry)) {
            m_cache.clear();
        } else {
            m_cache.insert(entry, true);
        }
        if (save) {
            m_userDict.save();
        }
    }
    emit userDictionaryChanged();
    return true;
}

/**! @brief List the dictionaries available to the engine.
 *
 * Each entry has the language tag and a display name, sorted by name.
 */
QList<SpellChecker::Language> SpellChecker::listDictionaries() const
{
    QList<Language> languages;
    const QList<DictionaryInfo> dicts = NuspellEngine::availableDictionaries(m_dictPaths);
    for (const DictionaryInfo &dict : dicts) {
        languages.append({dict.tag, languageName(dict.tag)});
    }
    std::sort(languages.begin(), languages.end(), [](const Language &a, const Language &b) {
        return a.name < b.name;
    });
    return languages;
}

// Static Methods
// ==============

/**! @brief Return a display name for a language tag, like "English (United Kingdom) [en_GB]".
 *
 * Falls back to the tag alone when Qt does not recognise it.
 */
QString SpellChecker::languageName(const QString &tag)
{
    QLocale locale(tag);
    if (locale.language() == QLocale::C || locale.language() == QLocale::AnyLanguage) {
        return tag;
    }
    QString name = QLocale::languageToString(locale.language());
    if (locale.territory() != QLocale::AnyTerritory) {
        name += QStringLiteral(" (%1)").arg(QLocale::territoryToString(locale.territory()));
    }
    return QStringLiteral("%1 [%2]").arg(name, tag);
}

// Setters
// =======

/**! @brief Point the user dictionary at a project's storage and load it.
 *
 * Call this when a project is opened or closed. Passing a null pointer just
 * clears the user words.
 */
void SpellChecker::setStorage(Storage *store)
{
    {
        QMutexLocker locker(&m_mutex);
        m_userDict.setStorage(store);
        m_userDict.load();
        m_cache.clear();
    }
    emit userDictionaryChanged();
}

/**! @brief Load the dictionary for a language.
 *
 * If it cannot be loaded, the null engine takes over so that every word is
 * accepted rather than rejected. An empty language unloads the engine.
 */
void SpellChecker::setLanguage(const QString &language)
{
    // The dictionary is loaded outside the lock, since it can take a moment
    // and the workers can keep using the old engine meanwhile.
    std::unique_ptr<SpellEngine> engine;
    if (language.isEmpty()) {
        engine = std::make_unique<NullSpellEngine>();
    } else {
        auto nuspell = std::make_unique<NuspellEngine>();
        if (nuspell->loadLanguage(language, m_dictPaths)) {
            qInfo() << "Spell checking for language" << language << "loaded";
            engine = std::move(nuspell);
        } else {
            qWarning() << "No spell checking available for language" << language;
            engine = std::make_unique<NullSpellEngine>();
        }
    }

    QString loaded;
    {
        QMutexLocker locker(&m_mutex);
        m_requested = language;
        m_cache.clear();
        m_engine = std::move(engine);
        loaded = m_engine->language();
    }
    emit languageChanged(loaded);
}

/**! @brief Set extra directories to search for dictionaries.
 *
 * These are searched before the system locations. Takes effect on the next
 * call to setLanguage() or listDictionaries().
 */
void SpellChecker::setDictionaryPaths(const QStringList &paths)
{
    m_dictPaths = paths;
}

} // namespace Collett
