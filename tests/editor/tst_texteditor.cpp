/*
** Collett - Text Editor Tests
** ===========================
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
#include "spellchecker.h"
#include "textblock.h"
#include "texteditor.h"
#include "theme.h"

#include <QAction>
#include <QMenu>
#include <QTextBlock>
#include <QTextCursor>
#include <QtTest>

using namespace Collett;

static const QString DATA_DIR = QStringLiteral(TEST_DATA_DIR);
static const QString TEST_TAG = QStringLiteral("xx_TEST");

class TestTextEditor : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void visibleBlocksCheckedAtOnce();
    void editsAreRechecked();
    void backgroundPass();
    void cursorWordNotMarked();
    void contextMenuSuggestions();
    void contextMenuAddWord();
    void noDocument();

private:
    TextBlockData *blockData(const QTextBlock &block) const;
    int selectionCount(const GuiTextEditor &editor, QTextCharFormat::UnderlineStyle style) const;
};

void TestTextEditor::initTestCase()
{
    QCoreApplication::setOrganizationName("CollettTest");
    QCoreApplication::setApplicationName("tst_texteditor");
}

void TestTextEditor::cleanupTestCase()
{
    Theme::destroy();
}

TextBlockData *TestTextEditor::blockData(const QTextBlock &block) const
{
    return dynamic_cast<TextBlockData *>(block.userData());
}

/**! @brief Count the extra selections drawn with a given underline style.
 */
int TestTextEditor::selectionCount(const GuiTextEditor &editor, QTextCharFormat::UnderlineStyle style) const
{
    int count = 0;
    const QList<QTextEdit::ExtraSelection> selections = editor.extraSelections();
    for (const QTextEdit::ExtraSelection &selection : selections) {
        if (selection.format.underlineStyle() == style) {
            count++;
        }
    }
    return count;
}

/**! @brief Opening a document checks the visible blocks and marks them at once.
 */
void TestTextEditor::visibleBlocksCheckedAtOnce()
{
    SpellChecker spell;
    spell.setDictionaryPaths({DATA_DIR});
    spell.setLanguage(TEST_TAG);

    GuiTextEditor editor;
    editor.resize(500, 400);
    editor.show();
    editor.setSpellChecker(&spell);

    Document doc;
    QTextCursor cursor(&doc);
    cursor.insertText("hello helo  world ");
    editor.openDocument(&doc);

    TextBlockData *data = blockData(doc.firstBlock());
    QVERIFY(data != nullptr);
    QCOMPARE(data->spellErrors().size(), 1);
    QCOMPARE(data->spellErrors().first().text, QStringLiteral("helo"));
    QCOMPARE(data->formatErrors().size(), 2);
    QCOMPARE(data->formatErrors().at(0).text, QStringLiteral("multi"));
    QCOMPARE(data->formatErrors().at(1).text, QStringLiteral("trail"));

    // Cursor is at the start, so nothing is suppressed
    QCOMPARE(selectionCount(editor, QTextCharFormat::SpellCheckUnderline), 1);
    QCOMPARE(selectionCount(editor, QTextCharFormat::SingleUnderline), 2);

    editor.openDocument(nullptr);
}

/**! @brief An edit clears the block's markers and rechecks it in the background.
 */
void TestTextEditor::editsAreRechecked()
{
    SpellChecker spell;
    spell.setDictionaryPaths({DATA_DIR});
    spell.setLanguage(TEST_TAG);

    GuiTextEditor editor;
    editor.resize(500, 400);
    editor.show();
    editor.setSpellChecker(&spell);

    Document doc;
    QTextCursor cursor(&doc);
    cursor.insertText("hello world");
    editor.openDocument(&doc);

    TextBlockData *data = blockData(doc.firstBlock());
    QVERIFY(data != nullptr);
    QVERIFY(data->spellErrors().isEmpty());
    int revision = data->revision();

    // Type a misspelled word, then move the cursor away from it
    QTextCursor edit = editor.textCursor();
    edit.movePosition(QTextCursor::End);
    edit.insertText(" wrld");
    edit.movePosition(QTextCursor::Start);
    editor.setTextCursor(edit);

    QCOMPARE(data->revision(), revision + 1);
    QVERIFY(data->spellErrors().isEmpty()); // Cleared, pending the recheck

    QTRY_COMPARE_WITH_TIMEOUT(data->spellErrors().size(), 1, 5000);
    QCOMPARE(data->spellErrors().first(), TextCheck({12, 16, "wrld"}));
    QTRY_COMPARE_WITH_TIMEOUT(selectionCount(editor, QTextCharFormat::SpellCheckUnderline), 1, 5000);

    // Fixing the word clears the marker again
    edit.movePosition(QTextCursor::End);
    edit.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
    edit.insertText("world");
    edit.movePosition(QTextCursor::Start);
    editor.setTextCursor(edit);
    QTRY_COMPARE_WITH_TIMEOUT(selectionCount(editor, QTextCharFormat::SpellCheckUnderline), 0, 5000);
    QTRY_VERIFY_WITH_TIMEOUT(data->revision() > revision + 1 && data->spellErrors().isEmpty(), 5000);

    editor.openDocument(nullptr);
}

/**! @brief Blocks below the visible area are checked by the background pass.
 */
void TestTextEditor::backgroundPass()
{
    SpellChecker spell;
    spell.setDictionaryPaths({DATA_DIR});
    spell.setLanguage(TEST_TAG);

    GuiTextEditor editor;
    editor.resize(500, 300);
    editor.show();
    editor.setSpellChecker(&spell);

    // More blocks than one job holds, and more than fit in the view
    Document doc;
    QTextCursor cursor(&doc);
    for (int i = 0; i < 250; ++i) {
        if (i > 0) {
            cursor.insertBlock();
        }
        // Only words from the test dictionary, plus one misspelling
        cursor.insertText(QStringLiteral("hello world %1 helo world").arg(i));
    }
    editor.openDocument(&doc);

    QTextBlock lastBlock = doc.lastBlock();
    TextBlockData *data = blockData(lastBlock);
    QVERIFY(data != nullptr);
    QTRY_COMPARE_WITH_TIMEOUT(data->spellErrors().size(), 1, 10000);
    QCOMPARE(data->spellErrors().first().text, QStringLiteral("helo"));

    // Every block ends up checked
    for (QTextBlock block = doc.firstBlock(); block.isValid(); block = block.next()) {
        TextBlockData *blk = blockData(block);
        QVERIFY(blk != nullptr);
        QTRY_COMPARE_WITH_TIMEOUT(blk->spellErrors().size(), 1, 10000);
    }

    editor.openDocument(nullptr);
}

/**! @brief The word under the cursor is not marked while it is being typed.
 */
void TestTextEditor::cursorWordNotMarked()
{
    SpellChecker spell;
    spell.setDictionaryPaths({DATA_DIR});
    spell.setLanguage(TEST_TAG);

    GuiTextEditor editor;
    editor.resize(500, 400);
    editor.show();
    editor.setSpellChecker(&spell);

    Document doc;
    QTextCursor cursor(&doc);
    cursor.insertText("helo world ");
    editor.openDocument(&doc);
    QCOMPARE(selectionCount(editor, QTextCharFormat::SpellCheckUnderline), 1);
    QCOMPARE(selectionCount(editor, QTextCharFormat::SingleUnderline), 1);

    // Cursor inside the misspelled word hides its marker
    QTextCursor move = editor.textCursor();
    move.setPosition(2);
    editor.setTextCursor(move);
    QTRY_COMPARE_WITH_TIMEOUT(selectionCount(editor, QTextCharFormat::SpellCheckUnderline), 0, 5000);
    QCOMPARE(selectionCount(editor, QTextCharFormat::SingleUnderline), 1);

    // Cursor at the trailing space hides that marker too
    move.movePosition(QTextCursor::End);
    editor.setTextCursor(move);
    QTRY_COMPARE_WITH_TIMEOUT(selectionCount(editor, QTextCharFormat::SingleUnderline), 0, 5000);
    QCOMPARE(selectionCount(editor, QTextCharFormat::SpellCheckUnderline), 1);

    editor.openDocument(nullptr);
}

/**! @brief Find a menu action by its text, or return null.
 */
static QAction *findAction(const QMenu *menu, const QString &text)
{
    const QList<QAction *> actions = menu->actions();
    for (QAction *action : actions) {
        if (action->text() == text) {
            return action;
        }
    }
    return nullptr;
}

/**! @brief The context menu on a misspelled word offers suggestions that replace it.
 */
void TestTextEditor::contextMenuSuggestions()
{
    SpellChecker spell;
    spell.setDictionaryPaths({DATA_DIR});
    spell.setLanguage(TEST_TAG);

    GuiTextEditor editor;
    editor.resize(500, 400);
    editor.show();
    editor.setSpellChecker(&spell);

    Document doc;
    QTextCursor cursor(&doc);
    cursor.insertText("hello helo");
    editor.openDocument(&doc);

    // On the misspelled word
    QTextCursor inWord(&doc);
    inWord.setPosition(8);
    QMenu *menu = editor.buildContextMenu(editor.cursorRect(inWord).center());
    QVERIFY(findAction(menu, "Add Word to Dictionary") != nullptr);
    QVERIFY(findAction(menu, "Ignore Word") != nullptr);
    QVERIFY(findAction(menu, "Spelling Suggestion(s)") != nullptr);
    QVERIFY(!findAction(menu, "Spelling Suggestion(s)")->isEnabled());

    QAction *suggestion = findAction(menu, QString::fromUtf8("– hello"));
    QVERIFY(suggestion != nullptr);
    suggestion->trigger();
    QCOMPARE(doc.toPlainText(), QStringLiteral("hello hello"));
    QCOMPARE(editor.textCursor().position(), 6);
    delete menu;

    // On a correct word there is no spelling section
    QTextCursor inGood(&doc);
    inGood.setPosition(2);
    menu = editor.buildContextMenu(editor.cursorRect(inGood).center());
    QVERIFY(findAction(menu, "Add Word to Dictionary") == nullptr);
    QVERIFY(findAction(menu, "Ignore Word") == nullptr);
    delete menu;

    editor.openDocument(nullptr);
}

/**! @brief Ignoring or adding a word removes its marker via a recheck.
 */
void TestTextEditor::contextMenuAddWord()
{
    SpellChecker spell;
    spell.setDictionaryPaths({DATA_DIR});
    spell.setLanguage(TEST_TAG);

    GuiTextEditor editor;
    editor.resize(500, 400);
    editor.show();
    editor.setSpellChecker(&spell);

    Document doc;
    QTextCursor cursor(&doc);
    cursor.insertText("hello wrld helo");
    editor.openDocument(&doc);
    QCOMPARE(selectionCount(editor, QTextCharFormat::SpellCheckUnderline), 2);

    // Ignore the first word for the session
    QTextCursor first(&doc);
    first.setPosition(7);
    QMenu *menu = editor.buildContextMenu(editor.cursorRect(first).center());
    QAction *ignore = findAction(menu, "Ignore Word");
    QVERIFY(ignore != nullptr);
    ignore->trigger();
    delete menu;
    QVERIFY(spell.checkWord("wrld"));
    QCOMPARE(selectionCount(editor, QTextCharFormat::SpellCheckUnderline), 1);

    // Add the second word to the dictionary
    QTextCursor second(&doc);
    second.setPosition(13);
    menu = editor.buildContextMenu(editor.cursorRect(second).center());
    QAction *add = findAction(menu, "Add Word to Dictionary");
    QVERIFY(add != nullptr);
    add->trigger();
    delete menu;
    QVERIFY(spell.checkWord("helo"));
    QCOMPARE(selectionCount(editor, QTextCharFormat::SpellCheckUnderline), 0);
    QCOMPARE(spell.userDictionary()->count(), 2);

    editor.openDocument(nullptr);
}

/**! @brief With no document open there are no markers and no crashes.
 */
void TestTextEditor::noDocument()
{
    GuiTextEditor editor;
    editor.resize(500, 400);
    editor.show();
    editor.openDocument(nullptr);
    QVERIFY(editor.extraSelections().isEmpty());
    editor.beginCheckPass();
    QVERIFY(editor.extraSelections().isEmpty());
    QVERIFY(!editor.isEnabled());
}

QTEST_MAIN(TestTextEditor)
#include "tst_texteditor.moc"
