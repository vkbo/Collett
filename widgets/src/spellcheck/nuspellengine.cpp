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

#include "nuspellengine.h"

#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QSet>

#include <exception>
#include <filesystem>
#include <string>
#include <vector>

#include <nuspell/dictionary.hxx>
#include <nuspell/finder.hxx>

namespace Collett {

struct NuspellEngine::Private
{
    nuspell::Dictionary dict;
};

/**! @brief Convert a Qt path to a std::filesystem path.
 *
 * Qt keeps paths as UTF-16, while the filesystem library wants them in the
 * native narrow encoding. Going through UTF-8 is correct on all the platforms
 * Collett targets.
 */
static std::filesystem::path toFsPath(const QString &path)
{
    return std::filesystem::path(path.toUtf8().toStdString());
}

/**! @brief Convert a std::filesystem path to a Qt path.
 */
static QString fromFsPath(const std::filesystem::path &path)
{
    return QString::fromUtf8(path.u8string().c_str());
}

/**! @brief Build the list of directories to search for dictionaries.
 *
 * The extra directories are searched before Nuspell's own defaults, so a
 * dictionary shipped with or installed by Collett takes precedence over one
 * from the system.
 */
static std::vector<std::filesystem::path> searchDirs(const QStringList &extraDirs)
{
    std::vector<std::filesystem::path> dirs;
    for (const QString &dir : extraDirs) {
        if (!dir.isEmpty()) {
            dirs.push_back(toFsPath(dir));
        }
    }
    nuspell::append_default_dir_paths(dirs);
    return dirs;
}

// Constructor/Destructor
// ======================

NuspellEngine::NuspellEngine() : d(std::make_unique<Private>()) {}

NuspellEngine::~NuspellEngine() = default;

// Public Methods
// ==============

/**! @brief Load the dictionary for a language tag.
 *
 * The tag is matched against the file stem of the .aff files found in the
 * search directories, so "en_GB" loads the first en_GB.aff found. Returns
 * false and leaves the engine unloaded if no such dictionary exists or it
 * fails to load.
 */
bool NuspellEngine::loadLanguage(const QString &tag, const QStringList &extraDirs)
{
    this->unload();
    if (tag.isEmpty()) {
        return false;
    }

    std::filesystem::path affPath;
    try {
        affPath = nuspell::search_dirs_for_one_dict(searchDirs(extraDirs), toFsPath(tag));
    } catch (const std::exception &e) {
        qWarning() << "Error searching for dictionary" << tag << ":" << e.what();
        return false;
    }
    if (affPath.empty()) {
        qWarning() << "Nuspell found no dictionary for language" << tag;
        return false;
    }
    return this->loadDictionary(fromFsPath(affPath), tag);
}

/**! @brief Load a dictionary directly from its .aff file.
 *
 * The matching .dic file must sit next to it. The tag is what the engine will
 * report as its language, and defaults to the file stem. Returns false and
 * leaves the engine unloaded on failure.
 */
bool NuspellEngine::loadDictionary(const QString &affPath, const QString &tag)
{
    this->unload();
    try {
        d->dict.load_aff_dic(toFsPath(affPath));
    } catch (const std::exception &e) {
        qWarning() << "Failed to load dictionary" << affPath << ":" << e.what();
        d->dict = nuspell::Dictionary();
        return false;
    }
    m_isLoaded = true;
    m_language = tag.isEmpty() ? QFileInfo(affPath).completeBaseName() : tag;
    qDebug() << "Loaded dictionary:" << affPath;
    return true;
}

/**! @brief Drop the loaded dictionary, if any.
 */
void NuspellEngine::unload()
{
    if (m_isLoaded) {
        d->dict = nuspell::Dictionary();
    }
    m_isLoaded = false;
    m_language.clear();
}

// Static Methods
// ==============

/**! @brief List the dictionaries available in the search directories.
 *
 * Extra directories are searched first and take precedence when the same
 * language tag appears more than once. The result is sorted by tag.
 */
QList<DictionaryInfo> NuspellEngine::availableDictionaries(const QStringList &extraDirs)
{
    std::vector<std::filesystem::path> found;
    try {
        nuspell::search_dirs_for_dicts(searchDirs(extraDirs), found);
    } catch (const std::exception &e) {
        qWarning() << "Error searching for dictionaries:" << e.what();
        return QList<DictionaryInfo>();
    }

    QList<DictionaryInfo> dicts;
    QSet<QString> seen;
    for (const std::filesystem::path &affPath : found) {
        QString tag = fromFsPath(affPath.stem());
        if (!seen.contains(tag)) {
            seen.insert(tag);
            dicts.append({tag, fromFsPath(affPath)});
        }
    }
    std::sort(dicts.begin(), dicts.end(), [](const DictionaryInfo &a, const DictionaryInfo &b) {
        return a.tag < b.tag;
    });
    return dicts;
}

// SpellEngine Interface
// =====================

bool NuspellEngine::spell(const QString &word) const
{
    if (!m_isLoaded) {
        return true;
    }
    QByteArray utf8 = word.toUtf8();
    return d->dict.spell(std::string_view(utf8.constData(), utf8.size()));
}

QStringList NuspellEngine::suggest(const QString &word) const
{
    QStringList result;
    if (m_isLoaded) {
        QByteArray utf8 = word.toUtf8();
        std::vector<std::string> out;
        d->dict.suggest(std::string_view(utf8.constData(), utf8.size()), out);
        result.reserve(out.size());
        for (const std::string &s : out) {
            result.append(QString::fromUtf8(s.c_str(), s.size()));
        }
    }
    return result;
}

} // namespace Collett
