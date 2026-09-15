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

#include "userdictionary.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include <algorithm>

using namespace Qt::Literals::StringLiterals;

namespace Collett {

// Constructor/Destructor
// ======================

UserDictionary::UserDictionary(Storage *store) : m_store(store) {}

// Public Methods
// ==============

/**! @brief Check whether a word is accepted by the user dictionary.
 *
 * A word is accepted if it is listed exactly, or if it starts with one of the
 * stems. The stem list is only scanned when the exact lookup fails, and the
 * caller only gets here for words the language dictionary rejected, so the
 * linear scan costs nothing on ordinary text.
 */
bool UserDictionary::contains(const QString &word) const
{
    if (m_words.contains(word)) {
        return true;
    }
    for (const QString &stem : m_stems) {
        if (word.startsWith(stem)) {
            return true;
        }
    }
    return false;
}

/**! @brief Add an entry to the dictionary.
 *
 * An entry ending in an asterisk is stored as a stem. Surrounding whitespace
 * is stripped. Returns true if the entry was added, and false if it was
 * already present or was empty.
 */
bool UserDictionary::add(const QString &entry)
{
    QString text = entry.trimmed();
    if (isStem(text)) {
        text.chop(1);
        if (text.isEmpty() || m_stems.contains(text)) {
            return false;
        }
        m_stems.append(text);
        return true;
    }
    if (text.isEmpty() || m_words.contains(text)) {
        return false;
    }
    m_words.insert(text);
    return true;
}

/**! @brief Remove an entry from the dictionary.
 *
 * The entry must be given the way it was added, so a stem keeps its trailing
 * asterisk. Returns true if something was removed.
 */
bool UserDictionary::remove(const QString &entry)
{
    QString text = entry.trimmed();
    if (isStem(text)) {
        text.chop(1);
        return m_stems.removeOne(text);
    }
    return m_words.remove(text);
}

void UserDictionary::clear()
{
    m_words.clear();
    m_stems.clear();
}

/**! @brief Load the word list from the project storage.
 *
 * A missing file is not an error, it just means the project has no user
 * words yet. Returns false only if there is no storage or the file could not
 * be read.
 */
bool UserDictionary::load()
{
    this->clear();
    if (m_store == nullptr) {
        return false;
    }
    QJsonObject data;
    if (!m_store->readDictionary(data)) {
        qWarning() << "Failed to load user dictionary";
        return false;
    }
    const QJsonArray words = data.value("c:words"_L1).toArray();
    for (const QJsonValue &value : words) {
        this->add(value.toString());
    }
    qDebug() << "Loaded user dictionary with" << this->count() << "entries";
    return true;
}

/**! @brief Save the word list to the project storage.
 */
bool UserDictionary::save() const
{
    if (m_store == nullptr) {
        return false;
    }
    QJsonObject data;
    data["c:format"_L1] = "CollettUserDictionary";
    data["c:words"_L1] = QJsonArray::fromStringList(this->entries());
    if (!m_store->writeDictionary(data)) {
        qWarning() << "Failed to save user dictionary";
        return false;
    }
    return true;
}

// Static Methods
// ==============

/**! @brief Check whether an entry is written as a stem, that is ends in "*".
 */
bool UserDictionary::isStem(const QString &entry)
{
    return entry.endsWith(u'*');
}

// Getters
// =======

/**! @brief Return all entries, sorted, with stems marked by a trailing "*".
 */
QStringList UserDictionary::entries() const
{
    QStringList result(m_words.cbegin(), m_words.cend());
    for (const QString &stem : m_stems) {
        result.append(stem + u'*');
    }
    std::sort(result.begin(), result.end());
    return result;
}

} // namespace Collett
