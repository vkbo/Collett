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

#include "highlighter.h"
#include "spellchecker.h"

#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextLayout>
#include <QtTest>

using namespace Collett;

static const QString DATA_DIR = QStringLiteral(TEST_DATA_DIR);

class TestHighlighter : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void marksMisspelledWords();
    void leavesDocumentUnchanged();
    void followsEdits();
    void userDictionary();
    void errorColor();
    void marksRedundantSpaces();
    void trailingSpaceAtCursor();

private:
    SpellChecker *m_spell = nullptr;

    QStringList marked(const QTextBlock &block) const;
    QList<QPair<int, int>> spaceMarks(const QTextBlock &block) const;
};

void TestHighlighter::init()
{
    m_spell = new SpellChecker();
    m_spell->setDictionaryPaths({DATA_DIR});
    m_spell->setLanguage("xx_TEST");
    QVERIFY(m_spell->isLoaded());
}

void TestHighlighter::cleanup()
{
    delete m_spell;
    m_spell = nullptr;
}

/**! @brief The text of each range of a block with a spell check underline.
 *
 * A highlighter checks a new document on the next pass of the event loop,
 * so the tests process events after setting the document.
 */
QStringList TestHighlighter::marked(const QTextBlock &block) const
{
    QStringList words;
    const QList<QTextLayout::FormatRange> formats = block.layout()->formats();
    for (const QTextLayout::FormatRange &range : formats) {
        if (range.format.underlineStyle() == QTextCharFormat::SpellCheckUnderline) {
            words.append(block.text().mid(range.start, range.length));
        }
    }
    return words;
}

/**! @brief The start and length of each range of a block with a plain
 * underline, which marks redundant spaces.
 */
QList<QPair<int, int>> TestHighlighter::spaceMarks(const QTextBlock &block) const
{
    QList<QPair<int, int>> ranges;
    const QList<QTextLayout::FormatRange> formats = block.layout()->formats();
    for (const QTextLayout::FormatRange &range : formats) {
        if (range.format.underlineStyle() == QTextCharFormat::SingleUnderline) {
            ranges.append({range.start, range.length});
        }
    }
    return ranges;
}

/**! @brief Misspelled words are underlined, and nothing is without a spell
 * checker.
 */
void TestHighlighter::marksMisspelledWords()
{
    QTextDocument doc;
    doc.setPlainText("hello helo world\nwrld Frodo");
    Highlighter highlighter;
    highlighter.setSpellChecker(m_spell);
    highlighter.setDocument(&doc);
    QCoreApplication::processEvents();

    QCOMPARE(marked(doc.firstBlock()), QStringList({"helo"}));
    QCOMPARE(marked(doc.lastBlock()), QStringList({"wrld"}));

    highlighter.setSpellChecker(nullptr);
    QCOMPARE(marked(doc.firstBlock()), QStringList());
}

/**! @brief The underlines are layout overlays, so the document is neither
 * modified nor given an undo step.
 */
void TestHighlighter::leavesDocumentUnchanged()
{
    QTextDocument doc;
    doc.setPlainText("helo wrld");
    doc.setModified(false);
    doc.clearUndoRedoStacks();

    Highlighter highlighter;
    highlighter.setSpellChecker(m_spell);
    highlighter.setDocument(&doc);
    QCoreApplication::processEvents();
    QCOMPARE(marked(doc.firstBlock()).size(), 2);
    QVERIFY(!doc.isModified());
    QVERIFY(!doc.isUndoAvailable());
    QCOMPARE(doc.firstBlock().charFormat().underlineStyle(), QTextCharFormat::NoUnderline);
}

/**! @brief Edited text is checked again. A document only reports edits
 * once it has a layout, which the editor always gives it.
 */
void TestHighlighter::followsEdits()
{
    QTextDocument doc;
    doc.documentLayout();
    doc.setPlainText("hello world");
    Highlighter highlighter;
    highlighter.setSpellChecker(m_spell);
    highlighter.setDocument(&doc);
    QCoreApplication::processEvents();
    QCOMPARE(marked(doc.firstBlock()), QStringList());

    QTextCursor cursor(&doc);
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(" wrld");
    QCOMPARE(marked(doc.firstBlock()), QStringList({"wrld"}));
}

/**! @brief Adding a word to the user dictionary removes its underline.
 */
void TestHighlighter::userDictionary()
{
    QTextDocument doc;
    doc.setPlainText("helo wrld");
    Highlighter highlighter;
    highlighter.setSpellChecker(m_spell);
    highlighter.setDocument(&doc);
    QCoreApplication::processEvents();
    QCOMPARE(marked(doc.firstBlock()), QStringList({"helo", "wrld"}));

    QVERIFY(m_spell->addWord("wrld", false));
    QCOMPARE(marked(doc.firstBlock()), QStringList({"helo"}));
}

/**! @brief The underline colour can be changed.
 */
void TestHighlighter::errorColor()
{
    QTextDocument doc;
    doc.setPlainText("helo");
    Highlighter highlighter;
    highlighter.setSpellChecker(m_spell);
    highlighter.setDocument(&doc);
    QCoreApplication::processEvents();
    highlighter.setErrorColor(QColor("#0000ff"));

    const QList<QTextLayout::FormatRange> formats = doc.firstBlock().layout()->formats();
    QCOMPARE(formats.size(), 1);
    QCOMPARE(formats.first().format.underlineColor(), QColor("#0000ff"));
}

/**! @brief When turned on, runs of spaces and trailing spaces are underlined
 * in their own colour, with or without a spell checker, alongside misspelled
 * words.
 */
void TestHighlighter::marksRedundantSpaces()
{
    QTextDocument doc;
    doc.setPlainText("hello  helo world \nwrld");
    Highlighter highlighter;
    highlighter.setFormatErrorColor(Qt::blue);
    highlighter.setDocument(&doc);
    QCoreApplication::processEvents();

    using Ranges = QList<QPair<int, int>>;
    QCOMPARE(spaceMarks(doc.firstBlock()), Ranges());

    highlighter.setCheckFormat(true);
    QCOMPARE(spaceMarks(doc.firstBlock()), Ranges({{5, 2}, {17, 1}}));
    QCOMPARE(spaceMarks(doc.lastBlock()), Ranges());
    for (const QTextLayout::FormatRange &range : doc.firstBlock().layout()->formats()) {
        QCOMPARE(range.format.underlineColor(), QColor(Qt::blue));
    }

    highlighter.setSpellChecker(m_spell);
    QCOMPARE(spaceMarks(doc.firstBlock()), Ranges({{5, 2}, {17, 1}}));
    QCOMPARE(marked(doc.firstBlock()), QStringList({"helo"}));
}

/**! @brief A trailing space with the cursor in it, or right after it, is
 * not underlined, as it is still being typed. Runs of spaces always are.
 */
void TestHighlighter::trailingSpaceAtCursor()
{
    QTextDocument doc;
    doc.setPlainText("one  two \nthree ");
    Highlighter highlighter;
    highlighter.setCheckFormat(true);
    highlighter.setDocument(&doc);
    QCoreApplication::processEvents();

    using Ranges = QList<QPair<int, int>>;
    const QTextBlock first = doc.firstBlock();
    const QTextBlock second = doc.lastBlock();
    QCOMPARE(spaceMarks(first), Ranges({{3, 2}, {8, 1}}));
    QCOMPARE(spaceMarks(second), Ranges({{5, 1}}));

    highlighter.setCursorPosition(9);
    QCOMPARE(spaceMarks(first), Ranges({{3, 2}}));
    QCOMPARE(spaceMarks(second), Ranges({{5, 1}}));

    highlighter.setCursorPosition(4);
    QCOMPARE(spaceMarks(first), Ranges({{3, 2}, {8, 1}}));

    highlighter.setCursorPosition(16);
    QCOMPARE(spaceMarks(first), Ranges({{3, 2}, {8, 1}}));
    QCOMPARE(spaceMarks(second), Ranges());

    highlighter.setCursorPosition(8);
    QCOMPARE(spaceMarks(first), Ranges({{3, 2}, {8, 1}}));
    QCOMPARE(spaceMarks(second), Ranges({{5, 1}}));
}

QTEST_MAIN(TestHighlighter)
#include "tst_highlighter.moc"
