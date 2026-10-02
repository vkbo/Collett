/*
** Collett - Document Highlighter Tests
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

#include "document.h"
#include "highlighter.h"
#include "textblock.h"
#include "theme.h"

#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextLayout>
#include <QtTest>

using namespace Collett;

class TestHighlighter : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void textBlockData();
    void blockColours();
    void storedColourWins();
    void blockDataFollowsEdits();

private:
    QList<QTextLayout::FormatRange> overlays(const QTextBlock &block) const;
};

/**! @brief Set app names so Settings and Theme can construct in isolation.
 */
void TestHighlighter::initTestCase()
{
    QCoreApplication::setOrganizationName("CollettTest");
    QCoreApplication::setApplicationName("tst_highlighter");
}

void TestHighlighter::cleanupTestCase()
{
    Theme::destroy();
}

/**! @brief Return the overlay formats the highlighter set on a block.
 */
QList<QTextLayout::FormatRange> TestHighlighter::overlays(const QTextBlock &block) const
{
    return block.layout() != nullptr ? block.layout()->formats() : QList<QTextLayout::FormatRange>();
}

/**! @brief Snapshots bump the revision and clear cached errors.
 */
void TestHighlighter::textBlockData()
{
    TextBlockData data;
    QCOMPARE(data.revision(), 0);
    QVERIFY(data.text().isEmpty());

    data.processText("Hello world");
    QCOMPARE(data.revision(), 1);
    QCOMPARE(data.text(), QStringLiteral("Hello world"));

    data.setSpellErrors({{6, 11, "world"}});
    data.setFormatErrors({{5, 6, "multi"}});
    QCOMPARE(data.spellErrors().size(), 1);
    QCOMPARE(data.spellErrors().first(), TextCheck({6, 11, "world"}));
    QCOMPARE(data.formatErrors().size(), 1);

    data.processText("Hello there");
    QCOMPARE(data.revision(), 2);
    QVERIFY(data.spellErrors().isEmpty());
    QVERIFY(data.formatErrors().isEmpty());

    data.setSpellErrors({{6, 11, "there"}});
    data.clear();
    QCOMPARE(data.revision(), 3);
    QVERIFY(data.text().isEmpty());
    QVERIFY(data.spellErrors().isEmpty());
}

/**! @brief Headings, comments and emphasis get the theme colours as overlays.
 */
void TestHighlighter::blockColours()
{
    Theme *theme = Theme::instance();

    Document doc;
    QTextCursor cursor(&doc);

    QTextBlockFormat heading;
    heading.setHeadingLevel(2);
    cursor.setBlockFormat(heading);
    cursor.insertText("Title");

    cursor.insertBlock(QTextBlockFormat());
    cursor.insertText("Body ");
    QTextCharFormat bold;
    bold.setFontWeight(QFont::Bold);
    cursor.insertText("bold", bold);
    cursor.insertText(" and ", QTextCharFormat());
    QTextCharFormat italic;
    italic.setFontItalic(true);
    cursor.insertText("italic", italic);

    QTextBlockFormat comment;
    comment.setProperty(BlockTypeProperty, CommentBlock);
    cursor.insertBlock(comment, QTextCharFormat());
    cursor.insertText("A note");

    GuiDocHighlighter highlighter;
    highlighter.setDocument(&doc);
    highlighter.rehighlight();

    // Heading block is coloured as a whole
    QTextBlock block = doc.firstBlock();
    QList<QTextLayout::FormatRange> ranges = overlays(block);
    QCOMPARE(ranges.size(), 1);
    QCOMPARE(ranges.at(0).start, 0);
    QCOMPARE(ranges.at(0).length, 5);
    QCOMPARE(ranges.at(0).format.foreground().color(), theme->getSyntaxColor(SyntaxHeader));

    // Only the bold and italic fragments of the paragraph are coloured
    block = block.next();
    ranges = overlays(block);
    QCOMPARE(ranges.size(), 2);
    QCOMPARE(ranges.at(0).start, 5);
    QCOMPARE(ranges.at(0).length, 4);
    QCOMPARE(ranges.at(0).format.foreground().color(), theme->getSyntaxColor(SyntaxEmphasis));
    QCOMPARE(ranges.at(1).start, 14);
    QCOMPARE(ranges.at(1).length, 6);
    QCOMPARE(ranges.at(1).format.foreground().color(), theme->getSyntaxColor(SyntaxEmphasis));

    // Comment block is coloured as a whole
    block = block.next();
    ranges = overlays(block);
    QCOMPARE(ranges.size(), 1);
    QCOMPARE(ranges.at(0).start, 0);
    QCOMPARE(ranges.at(0).length, 6);
    QCOMPARE(ranges.at(0).format.foreground().color(), theme->getSyntaxColor(SyntaxComment));

    // Every block got a text snapshot
    for (block = doc.firstBlock(); block.isValid(); block = block.next()) {
        TextBlockData *data = dynamic_cast<TextBlockData *>(block.userData());
        QVERIFY(data != nullptr);
        QCOMPARE(data->text(), block.text());
        QVERIFY(data->revision() >= 1);
    }
}

/**! @brief A fragment with its own foreground colour is left alone.
 */
void TestHighlighter::storedColourWins()
{
    Document doc;
    QTextCursor cursor(&doc);

    QTextCharFormat coloured;
    coloured.setFontWeight(QFont::Bold);
    coloured.setForeground(QColor(1, 2, 3));
    cursor.insertText("Said ", QTextCharFormat());
    cursor.insertText("loudly", coloured);

    GuiDocHighlighter highlighter;
    highlighter.setDocument(&doc);
    highlighter.rehighlight();

    QVERIFY(overlays(doc.firstBlock()).isEmpty());
}

/**! @brief Editing a block re-snapshots it and drops its cached errors.
 */
void TestHighlighter::blockDataFollowsEdits()
{
    Document doc;
    QTextCursor cursor(&doc);
    cursor.insertText("First");
    cursor.insertBlock();
    cursor.insertText("Second");

    // A document only emits contentsChange once it has a layout. The editor
    // creates one, but a bare document does not, so force it here.
    doc.documentLayout();

    GuiDocHighlighter highlighter;
    highlighter.setDocument(&doc);
    highlighter.rehighlight();

    QTextBlock second = doc.firstBlock().next();
    TextBlockData *data = dynamic_cast<TextBlockData *>(second.userData());
    QVERIFY(data != nullptr);
    int revision = data->revision();
    data->setSpellErrors({{0, 6, "Second"}});

    // Edit the second block only
    cursor.movePosition(QTextCursor::End);
    cursor.insertText("!");

    QCOMPARE(second.userData(), data); // Same data object survives the edit
    QCOMPARE(data->text(), QStringLiteral("Second!"));
    QCOMPARE(data->revision(), revision + 1);
    QVERIFY(data->spellErrors().isEmpty());

    // The first block was not touched
    TextBlockData *first = dynamic_cast<TextBlockData *>(doc.firstBlock().userData());
    QVERIFY(first != nullptr);
    QCOMPARE(first->text(), QStringLiteral("First"));
}

QTEST_MAIN(TestHighlighter)
#include "tst_highlighter.moc"
