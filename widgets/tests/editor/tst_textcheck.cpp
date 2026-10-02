/*
** Collett - Text Check Function Tests
** ===================================
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
#include "textblock.h"

#include <QtTest>

using namespace Collett;

static const QString DATA_DIR = QStringLiteral(TEST_DATA_DIR);
static const QString TEST_TAG = QStringLiteral("xx_TEST");

class TestTextCheck : public QObject
{
    Q_OBJECT

private slots:
    void spellCheck();
    void spellCheckSkips();
    void spellCheckSplitting();
    void formatCheck();
};

/**! @brief Misspelled words are returned with their positions.
 */
void TestTextCheck::spellCheck()
{
    SpellChecker spell;
    spell.setDictionaryPaths({DATA_DIR});
    spell.setLanguage(TEST_TAG);
    QVERIFY(spell.isLoaded());

    TextCheckList errors = spellCheckText("hello helo world wrld", &spell);
    QCOMPARE(errors.size(), 2);
    QCOMPARE(errors.at(0), TextCheck({6, 10, "helo"}));
    QCOMPARE(errors.at(1), TextCheck({17, 21, "wrld"}));

    QVERIFY(spellCheckText("", &spell).isEmpty());
    QVERIFY(spellCheckText("hello world", &spell).isEmpty());

    // No checker flags nothing
    QVERIFY(spellCheckText("helo wrld", nullptr).isEmpty());
}

/**! @brief Numbers and all upper case words are not checked.
 */
void TestTextCheck::spellCheckSkips()
{
    SpellChecker spell;
    spell.setDictionaryPaths({DATA_DIR});
    spell.setLanguage(TEST_TAG);

    TextCheckList errors = spellCheckText("hello 1234 NASA Frodo frodo", &spell);
    QCOMPARE(errors.size(), 1);
    QCOMPARE(errors.at(0).text, QStringLiteral("frodo"));

    // A modifier letter apostrophe is checked as a plain apostrophe, and the
    // reported text keeps the original characters
    errors = spellCheckText(QString::fromUtf8("helloʼs"), &spell);
    QCOMPARE(errors.size(), 1);
    QCOMPARE(errors.at(0).text, QString::fromUtf8("helloʼs"));
}

/**! @brief Hyphens, dashes and brackets split words, apostrophes do not.
 */
void TestTextCheck::spellCheckSplitting()
{
    SpellChecker spell;
    spell.setDictionaryPaths({DATA_DIR});
    spell.setLanguage(TEST_TAG);

    TextCheckList errors = spellCheckText(QString::fromUtf8("hello-helo world—wrld [hello] (helo)"), &spell);
    QCOMPARE(errors.size(), 3);
    QCOMPARE(errors.at(0), TextCheck({6, 10, "helo"}));
    QCOMPARE(errors.at(1), TextCheck({17, 21, "wrld"}));
    QCOMPARE(errors.at(2), TextCheck({31, 35, "helo"}));

    errors = spellCheckText("helo's", &spell);
    QCOMPARE(errors.size(), 1);
    QCOMPARE(errors.at(0), TextCheck({0, 6, "helo's"}));
}

/**! @brief Runs of spaces and trailing whitespace are flagged.
 */
void TestTextCheck::formatCheck()
{
    TextCheckList errors = formatCheckText("a  b   c ");
    QCOMPARE(errors.size(), 3);
    QCOMPARE(errors.at(0), TextCheck({1, 3, "multi"}));
    QCOMPARE(errors.at(1), TextCheck({4, 7, "multi"}));
    QCOMPARE(errors.at(2), TextCheck({8, 9, "trail"}));

    errors = formatCheckText("clean text\t");
    QCOMPARE(errors.size(), 1);
    QCOMPARE(errors.at(0), TextCheck({10, 11, "trail"}));

    QVERIFY(formatCheckText("clean text").isEmpty());
    QVERIFY(formatCheckText("").isEmpty());
}

QTEST_MAIN(TestTextCheck)
#include "tst_textcheck.moc"
