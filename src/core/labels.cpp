/*
** Collett - Common Labels
** =======================
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

#include "labels.h"

#include <QCoreApplication>
#include <QLocale>

using namespace Qt::Literals::StringLiterals;

namespace Collett {

namespace {

/**! @brief Use the English singular or plural when a count is not translated.
 *
 * The source text is British English, with "(s)" marking the plural, like
 * "%Ln word(s)". Translations give their own plural forms.
 */
QString englishPlural(const QString &translated, const QString &source, int count)
{
    QString untranslated = source;
    untranslated.replace("%Ln"_L1, QLocale().toString(count));
    if (translated != untranslated) return translated;
    return untranslated.replace("(s)"_L1, count == 1 ? ""_L1 : "s"_L1);
}

} // namespace

/**! @brief The name of a structure level.
 */
QString Labels::levelName(int level)
{
    switch (level) {
    case ItemLevel::PartitionLevel:
        return QCoreApplication::translate("Label", "Partition");
    case ItemLevel::ChapterLevel:
        return QCoreApplication::translate("Label", "Chapter");
    case ItemLevel::PageLevel:
        return QCoreApplication::translate("Label", "Page");
    default:
        return QCoreApplication::translate("Label", "Scene");
    }
}

/**! @brief The name of a project group class.
 */
QString Labels::className(int itemClass)
{
    switch (itemClass) {
    case ItemClass::NovelClass:
        return QCoreApplication::translate("Label", "Novel");
    case ItemClass::CharacterClass:
        //: The people of a story
        return QCoreApplication::translate("Label", "Characters");
    case ItemClass::PlotClass:
        return QCoreApplication::translate("Label", "Plot");
    case ItemClass::LocationClass:
        return QCoreApplication::translate("Label", "Locations");
    case ItemClass::ObjectClass:
        return QCoreApplication::translate("Label", "Objects");
    case ItemClass::EntityClass:
        return QCoreApplication::translate("Label", "Entities");
    case ItemClass::ArchiveClass:
        return QCoreApplication::translate("Label", "Archive");
    case ItemClass::TrashClass:
        return QCoreApplication::translate("Label", "Trash");
    default:
        return QCoreApplication::translate("Label", "Custom");
    }
}

/**! @brief The name of a text statistic.
 */
QString Labels::statName(Stat stat)
{
    switch (stat) {
    case Words:
        return QCoreApplication::translate("Stats", "Words");
    case Characters:
        //: Letters, digits, spaces and symbols in a text
        return QCoreApplication::translate("Stats", "Characters");
    case Paragraphs:
        return QCoreApplication::translate("Stats", "Paragraphs");
    }
    return QString();
}

/**! @brief The number of a document, or an empty string if it has none.
 *
 * Numbered chapters show their number, and scenes the number of their
 * chapter and their own, like "3.2". Partitions, pages and unnumbered
 * chapters have none.
 */
QString Labels::levelNumber(int level, bool numbered, int number, int chapterNumber)
{
    switch (level) {
    case ItemLevel::ChapterLevel:
        return numbered ? QString::number(number) : QString();
    case ItemLevel::SceneLevel:
        return chapterNumber > 0 ? u"%1.%2"_s.arg(chapterNumber).arg(number) : QString::number(number);
    default:
        return QString();
    }
}

/**! @brief A word count, like "1,234 words", in the plural form of the
 * language.
 */
QString Labels::wordCount(int words)
{
    return englishPlural(QCoreApplication::translate("Stats", "%Ln word(s)", nullptr, words), u"%Ln word(s)"_s, words);
}

} // namespace Collett
