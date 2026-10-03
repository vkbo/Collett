/*
** Collett - Labels Tests
** ======================
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

#include <QLocale>
#include <QtTest>

using namespace Collett;

class TestLabels : public QObject
{
    Q_OBJECT

private slots:
    void names();
    void wordCountPlurals();
    void levelNumbers();
};

void TestLabels::names()
{
    QCOMPARE(Labels::levelName(ItemLevel::ChapterLevel), QStringLiteral("Chapter"));
    QCOMPARE(Labels::levelName(ItemLevel::PageLevel), QStringLiteral("Page"));
    QCOMPARE(Labels::className(ItemClass::CharacterClass), QStringLiteral("Characters"));
    QCOMPARE(Labels::statName(Labels::Characters), QStringLiteral("Characters"));
}

/**! @brief Without a translation, word counts use the English singular
 * and plural, with the digits grouped by the locale.
 */
void TestLabels::wordCountPlurals()
{
    QLocale::setDefault(QLocale("en_GB"));
    QCOMPARE(Labels::wordCount(0), QStringLiteral("0 words"));
    QCOMPARE(Labels::wordCount(1), QStringLiteral("1 word"));
    QCOMPARE(Labels::wordCount(1234), QStringLiteral("1,234 words"));
}

/**! @brief Numbered chapters and scenes have numbers, and the rest have
 * none.
 */
void TestLabels::levelNumbers()
{
    QCOMPARE(Labels::levelNumber(ItemLevel::ChapterLevel, true, 3, 0), QStringLiteral("3"));
    QCOMPARE(Labels::levelNumber(ItemLevel::ChapterLevel, false, 3, 0), QString());
    QCOMPARE(Labels::levelNumber(ItemLevel::SceneLevel, false, 2, 3), QStringLiteral("3.2"));
    QCOMPARE(Labels::levelNumber(ItemLevel::SceneLevel, false, 2, 0), QStringLiteral("2"));
    QCOMPARE(Labels::levelNumber(ItemLevel::PartitionLevel, true, 1, 0), QString());
    QCOMPARE(Labels::levelNumber(ItemLevel::PageLevel, true, 1, 0), QString());
}

QTEST_GUILESS_MAIN(TestLabels)
#include "tst_labels.moc"
