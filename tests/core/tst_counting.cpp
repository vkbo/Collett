/*
** Collett - Text Counting Tests
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

#include "counting.h"

#include <QTextBlockFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QtTest>

using namespace Collett;

class TestCounting : public QObject
{
    Q_OBJECT

private slots:
    void countWords();
    void countCharacters();
    void standardCountBlocks();
    void standardCountText();
    void snapshotDocument();
};

/**! @brief Words are split on whitespace and dashes, but not hyphens.
 */
void TestCounting::countWords()
{
    QCOMPARE(TextCounter::countWords(""), 0);
    QCOMPARE(TextCounter::countWords("   "), 0);
    QCOMPARE(TextCounter::countWords("hello"), 1);
    QCOMPARE(TextCounter::countWords("hello world"), 2);
    QCOMPARE(TextCounter::countWords("  hello   world  "), 2);
    QCOMPARE(TextCounter::countWords("hello\tworld\nagain"), 3);
    QCOMPARE(TextCounter::countWords("well-known word"), 2);
    QCOMPARE(TextCounter::countWords("it's a word"), 3);
    QCOMPARE(TextCounter::countWords(QString::fromUtf8("one–two—three")), 3);
    QCOMPARE(TextCounter::countWords(QString::fromUtf8("dash — spaced")), 2);

    // A soft line break within a block separates words
    QCOMPARE(TextCounter::countWords(QStringLiteral("one") + QChar::LineSeparator + QStringLiteral("two")), 2);
}

/**! @brief Characters are counted without trailing whitespace.
 */
void TestCounting::countCharacters()
{
    QCOMPARE(TextCounter::countCharacters(""), 0);
    QCOMPARE(TextCounter::countCharacters("   "), 0);
    QCOMPARE(TextCounter::countCharacters("hello world"), 11);
    QCOMPARE(TextCounter::countCharacters("hello world  "), 11);
    QCOMPARE(TextCounter::countCharacters("  hello"), 7);
    QCOMPARE(TextCounter::countCharacters(QString::fromUtf8("blåbær")), 6);
}

/**! @brief The standard count skips comments and does not count headings as paragraphs.
 */
void TestCounting::standardCountBlocks()
{
    CountBlockList blocks;
    blocks.append({"Chapter One", true, false});
    blocks.append({"", false, false});
    blocks.append({"The first paragraph has seven words.", false, false});
    blocks.append({"A comment that is not counted", false, true});
    blocks.append({"Second paragraph. ", false, false});
    blocks.append({"   ", false, false});

    TextCounts counts = TextCounter::standardCount(blocks);
    QCOMPARE(counts.words, 2 + 6 + 2);
    QCOMPARE(counts.characters, 11 + 36 + 17);
    QCOMPARE(counts.paragraphs, 2);

    QCOMPARE(TextCounter::standardCount(CountBlockList()), TextCounts());
    QVERIFY(counts != TextCounts());
}

/**! @brief Plain text is counted with one paragraph per non-empty line.
 */
void TestCounting::standardCountText()
{
    TextCounts counts = TextCounter::standardCount("Hello world\n\nSecond line here\n");
    QCOMPARE(counts.words, 5);
    QCOMPARE(counts.characters, 11 + 16);
    QCOMPARE(counts.paragraphs, 2);

    QCOMPARE(TextCounter::standardCount(QString()), TextCounts());
}

/**! @brief A document snapshot carries the text and block type of each block.
 */
void TestCounting::snapshotDocument()
{
    QTextDocument doc;
    QTextCursor cursor(&doc);

    QTextBlockFormat heading;
    heading.setHeadingLevel(2);
    cursor.setBlockFormat(heading);
    cursor.insertText("Title");

    cursor.insertBlock(QTextBlockFormat());
    cursor.insertText("Body text here");

    QTextBlockFormat comment;
    comment.setProperty(BlockTypeProperty, CommentBlock);
    cursor.insertBlock(comment);
    cursor.insertText("A note");

    CountBlockList blocks = TextCounter::snapshot(&doc);
    QCOMPARE(blocks.size(), 3);
    QCOMPARE(blocks.at(0).text, QStringLiteral("Title"));
    QVERIFY(blocks.at(0).heading);
    QVERIFY(!blocks.at(0).comment);
    QCOMPARE(blocks.at(1).text, QStringLiteral("Body text here"));
    QVERIFY(!blocks.at(1).heading);
    QVERIFY(!blocks.at(1).comment);
    QCOMPARE(blocks.at(2).text, QStringLiteral("A note"));
    QVERIFY(!blocks.at(2).heading);
    QVERIFY(blocks.at(2).comment);

    TextCounts counts = TextCounter::standardCount(blocks);
    QCOMPARE(counts.words, 4);
    QCOMPARE(counts.characters, 5 + 14);
    QCOMPARE(counts.paragraphs, 1);

    QVERIFY(TextCounter::snapshot(nullptr).isEmpty());
}

QTEST_MAIN(TestCounting)
#include "tst_counting.moc"
