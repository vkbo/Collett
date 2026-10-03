/*
** Collett - Group Editor Tests
** ============================
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

#include "qmlfixture.h"
#include "settings.h"

#include <QTextBlock>
#include <QtTest>

using namespace Collett;

class TestGroupEditor : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void splitAtCursor();
    void splitCyclesType();
    void backspaceMerges();
    void backspaceRemovesEmpty();
    void arrowsCrossDocuments();
    void liveWordCount();
    void spellingMenu();
    void tabIndent();
    void autoIndent();
    void multiSpaces();
    void typingWord();
    void deleteFromTypeMenu();
    void emptyTextCursor();
    void goalColumn();
    void undoSplit();
    void undoDelete();
    void selectAcross();
    void typeTitle();
    void deleteAtEnd();
    void lineBreakInTitle();
    void firstTitleRemoved();

private:
    QmlFixture *f = nullptr;

    qreal indentAt(int row, int position) const;
    void setText(int row, const QString &text) const;
    QQuickItem *menuItem(QObject *menu, const QString &text) const;
};

/**! @brief The first-line indent of the paragraph at a position in the
 * text of a document.
 */
qreal TestGroupEditor::indentAt(int row, int position) const
{
    return f->doc()->findBlock(f->textPosition(row, position)).blockFormat().textIndent();
}

/**! @brief Replace the text of a document.
 */
void TestGroupEditor::setText(int row, const QString &text) const
{
    QTextCursor cursor(f->doc());
    cursor.setPosition(f->textPosition(row, 0));
    cursor.setPosition(f->doc()->itemEnd(f->handle(row)), QTextCursor::KeepAnchor);
    cursor.insertText(text);
}

/**! @brief The item of a menu with a given text.
 */
QQuickItem *TestGroupEditor::menuItem(QObject *menu, const QString &text) const
{
    const int count = menu->property("count").toInt();
    for (int i = 0; i < count; ++i) {
        QQuickItem *item = nullptr;
        QMetaObject::invokeMethod(menu, "itemAt", Q_RETURN_ARG(QQuickItem *, item), Q_ARG(int, i));
        if (item && item->property("text").toString() == text) return item;
    }
    return nullptr;
}

void TestGroupEditor::initTestCase()
{
    initQmlTests();
}

/**! @brief Every test starts from the same project: a title page, then
 * chapter 1 with one scene, and chapter 2 with two scenes.
 */
void TestGroupEditor::init()
{
    Settings::instance()->setTextAutoIndent(false);
    Settings::instance()->setShowMultiSpaces(false);
    f = new QmlFixture();
    QVERIFY(f->create());
    f->addDocument(ItemLevel::ChapterLevel, "One", "");
    f->addDocument(ItemLevel::SceneLevel, "", "Alpha beta.\nGamma delta.");
    f->addDocument(ItemLevel::ChapterLevel, "Two", "");
    f->addDocument(ItemLevel::SceneLevel, "", "Epsilon.");
    f->addDocument(ItemLevel::SceneLevel, "", "Zeta eta theta.");
    QVERIFY(f->load());
}

void TestGroupEditor::cleanup()
{
    delete f;
    f = nullptr;
}

/**! @brief Ctrl+Enter moves the text after the cursor into a new scene, and
 * puts the cursor in its title.
 */
void TestGroupEditor::splitAtCursor()
{
    f->enterText(2, 6);
    QTRY_COMPARE(f->focusHandle(), f->handle(2));

    QTest::keyClick(f->window, Qt::Key_Return, Qt::ControlModifier);
    QTRY_COMPARE(f->rows(), 7);
    QCOMPARE(f->text(2), QStringLiteral("Alpha "));
    QCOMPARE(f->text(3), QStringLiteral("beta.\nGamma delta."));
    QCOMPARE(f->value(3, ProjectModel::LevelRole).toInt(), int(ItemLevel::SceneLevel));
    QTRY_COMPARE(f->focusHandle(), f->handle(3));
    QCOMPARE(f->focusPart(), QStringLiteral("title"));
}

/**! @brief Ctrl+Enter in the title of a new document cycles its type, and
 * Enter moves on to its text.
 */
void TestGroupEditor::splitCyclesType()
{
    f->enterText(2, 6);
    QTest::keyClick(f->window, Qt::Key_Return, Qt::ControlModifier);
    QTRY_COMPARE(f->focusHandle(), f->handle(3));
    QTRY_COMPARE(f->focusPart(), QStringLiteral("title"));

    QTest::keyClick(f->window, Qt::Key_Return, Qt::ControlModifier);
    QTRY_COMPARE(f->value(3, ProjectModel::HardBreakRole).toBool(), true);

    QTest::keyClick(f->window, Qt::Key_Return, Qt::ControlModifier);
    QTRY_COMPARE(f->value(3, ProjectModel::LevelRole).toInt(), int(ItemLevel::ChapterLevel));
    QCOMPARE(f->value(3, ProjectModel::HardBreakRole).toBool(), false);

    QTest::keyClick(f->window, Qt::Key_Return, Qt::ControlModifier);
    QTRY_COMPARE(f->value(3, ProjectModel::LevelRole).toInt(), int(ItemLevel::SceneLevel));

    QTest::keyClick(f->window, Qt::Key_Return);
    QTRY_COMPARE(f->focusPart(), QStringLiteral("text"));
    QCOMPARE(f->focusHandle(), f->handle(3));
    QCOMPARE(f->focusCursor(), 0);
}

/**! @brief Backspace at the start of an untitled document merges it into
 * the one before, with the cursor where the merged text starts.
 */
void TestGroupEditor::backspaceMerges()
{
    const QString scene = f->handle(2);
    const QString next = f->handle(5);
    f->enterText(5, 0);
    QTRY_COMPARE(f->focusHandle(), next);

    QTest::keyClick(f->window, Qt::Key_Backspace);
    QTRY_COMPARE(f->rows(), 5);
    QCOMPARE(f->model()->rowOf(next), -1);
    QCOMPARE(f->text(4), QStringLiteral("Epsilon.\nZeta eta theta."));
    QTRY_COMPARE(f->focusHandle(), f->handle(4));
    QCOMPARE(f->focusPart(), QStringLiteral("text"));
    QCOMPARE(f->focusCursor(), f->text(4).indexOf("Zeta"));
    QCOMPARE(f->handle(2), scene);
}

/**! @brief An empty document made by Ctrl+Enter at the end of the text is
 * removed again with Backspace.
 */
void TestGroupEditor::backspaceRemovesEmpty()
{
    const int end = f->text(5).length();
    f->enterText(5, end);
    QTest::keyClick(f->window, Qt::Key_Return, Qt::ControlModifier);
    QTRY_COMPARE(f->rows(), 7);
    QTRY_COMPARE(f->focusHandle(), f->handle(6));
    QCOMPARE(f->text(6), QString());

    QTest::keyClick(f->window, Qt::Key_Backspace);
    QTRY_COMPARE(f->rows(), 6);
    QCOMPARE(f->text(5), QStringLiteral("Zeta eta theta."));
    QTRY_COMPARE(f->focusHandle(), f->handle(5));
    QCOMPARE(f->focusCursor(), end);
}

/**! @brief Down on the last line goes to the title of the next document,
 * and Up from a title goes back to the text above.
 */
void TestGroupEditor::arrowsCrossDocuments()
{
    f->enterText(2, f->text(2).length());
    QTRY_COMPARE(f->focusHandle(), f->handle(2));

    QTest::keyClick(f->window, Qt::Key_Down);
    QTRY_COMPARE(f->focusHandle(), f->handle(3));
    QCOMPARE(f->focusPart(), QStringLiteral("title"));

    QTest::keyClick(f->window, Qt::Key_Up);
    QTRY_COMPARE(f->focusHandle(), f->handle(2));
    QCOMPARE(f->focusPart(), QStringLiteral("text"));

    // Right at the end of the text also goes to the next title
    QTest::keyClick(f->window, Qt::Key_End);
    QTest::keyClick(f->window, Qt::Key_Right);
    QTRY_COMPARE(f->focusHandle(), f->handle(3));
    QCOMPARE(f->focusPart(), QStringLiteral("title"));
}

/**! @brief Typing updates the word count of the document shortly after.
 */
void TestGroupEditor::liveWordCount()
{
    QTRY_COMPARE(f->value(5, ProjectModel::WordsRole).toInt(), 3);
    f->enterText(5, f->text(5).length());
    QTRY_COMPARE(f->focusHandle(), f->handle(5));

    f->type(" Iota kappa");
    QCOMPARE(f->text(5), QStringLiteral("Zeta eta theta. Iota kappa"));
    QTRY_COMPARE(f->value(5, ProjectModel::WordsRole).toInt(), 5);
}

/**! @brief A right click on a misspelled word offers suggestions, which
 * replace the word, and adding the word to the dictionary.
 */
void TestGroupEditor::spellingMenu()
{
    SpellChecker *spell = f->project.spellChecker();
    spell->setDictionaryPaths({QStringLiteral(TEST_DATA_DIR)});
    spell->setLanguage("xx_TEST");
    QVERIFY(spell->isLoaded());

    setText(5, "hello wrld");
    QObject *menu = f->window->findChild<QObject *>("spellMenu");
    QVERIFY(menu);

    // A correct word has no menu
    QTest::mouseClick(f->window, Qt::RightButton, Qt::NoModifier, f->pointInText(5, 2));
    QTest::qWait(50);
    QVERIFY(!menu->property("opened").toBool());

    // Choosing a suggestion replaces the word
    QTest::mouseClick(f->window, Qt::RightButton, Qt::NoModifier, f->pointInText(5, 8));
    QTRY_VERIFY(menu->property("opened").toBool());
    QQuickItem *suggestion = menuItem(menu, "world");
    QVERIFY(suggestion);
    QTest::mouseClick(f->window, Qt::LeftButton, Qt::NoModifier, f->pointIn(suggestion));
    QTRY_COMPARE(f->text(5), QStringLiteral("hello world"));
    QTRY_VERIFY(!menu->property("visible").toBool());

    // Adding a word to the dictionary accepts it from then on
    setText(5, "hello Gollum");
    QVERIFY(!spell->checkWord("Gollum"));
    QTest::mouseClick(f->window, Qt::RightButton, Qt::NoModifier, f->pointInText(5, 8));
    QTRY_VERIFY(menu->property("opened").toBool());
    QQuickItem *add = menuItem(menu, "Add to Dictionary");
    QVERIFY(add);
    QTest::mouseClick(f->window, Qt::LeftButton, Qt::NoModifier, f->pointIn(add));
    QTRY_VERIFY(spell->checkWord("Gollum"));
    QCOMPARE(f->text(5), QStringLiteral("hello Gollum"));
}

/**! @brief Tab at the start of a paragraph adds a first-line indent, and
 * Backspace there removes it. Elsewhere, and in headings, Tab is a tab.
 */
void TestGroupEditor::tabIndent()
{
    const qreal width = Settings::instance()->textFormat().tabWidth;
    f->enterText(2, 12);
    QTRY_COMPARE(f->focusPart(), QStringLiteral("text"));

    QTest::keyClick(f->window, Qt::Key_Tab);
    QTRY_COMPARE(indentAt(2, 12), width);
    QCOMPARE(indentAt(2, 0), 0.0);
    QCOMPARE(f->text(2), QStringLiteral("Alpha beta.\nGamma delta."));

    QTest::keyClick(f->window, Qt::Key_Backspace);
    QTRY_COMPARE(indentAt(2, 12), 0.0);
    QCOMPARE(f->text(2), QStringLiteral("Alpha beta.\nGamma delta."));
    QCOMPARE(f->focusCursor(), 12);

    // Not at the start of a paragraph
    f->enterText(2, 5);
    QTRY_COMPARE(f->focusCursor(), 5);
    QTest::keyClick(f->window, Qt::Key_Tab);
    QTRY_COMPARE(f->text(2), QStringLiteral("Alpha\t beta.\nGamma delta."));
    QCOMPARE(indentAt(2, 0), 0.0);
}

/**! @brief With automatic indent on, Enter indents the new paragraph after
 * a text paragraph, but not after a heading or a centred paragraph.
 */
void TestGroupEditor::autoIndent()
{
    const qreal width = Settings::instance()->textFormat().tabWidth;
    QObject *binder = f->binder();
    QVERIFY(binder);

    // Off: the new paragraph copies the current one
    f->enterText(2, 11);
    QTRY_COMPARE(f->focusCursor(), 11);
    QTest::keyClick(f->window, Qt::Key_Return);
    QTRY_COMPARE(f->text(2), QStringLiteral("Alpha beta.\n\nGamma delta."));
    QCOMPARE(indentAt(2, 12), 0.0);

    // On: after a text paragraph
    Settings::instance()->setTextAutoIndent(true);
    QTest::keyClick(f->window, Qt::Key_Return);
    QTRY_COMPARE(f->text(2), QStringLiteral("Alpha beta.\n\n\nGamma delta."));
    QCOMPARE(indentAt(2, 13), width);
    QCOMPARE(indentAt(2, 12), 0.0);

    // After a heading
    f->enterText(2, 0);
    QTRY_COMPARE(f->focusCursor(), 0);
    binder->setProperty("headingLevel", 2);
    f->enterText(2, 11);
    QTRY_COMPARE(f->focusCursor(), 11);
    QTest::keyClick(f->window, Qt::Key_Return);
    f->type("x");
    QTRY_COMPARE(f->text(2), QStringLiteral("Alpha beta.\nx\n\n\nGamma delta."));
    QCOMPARE(indentAt(2, 12), 0.0);

    // After a centred paragraph
    binder->setProperty("alignment", int(Qt::AlignHCenter));
    QTest::keyClick(f->window, Qt::Key_Return);
    f->type("y");
    QTRY_COMPARE(f->text(2), QStringLiteral("Alpha beta.\nx\ny\n\n\nGamma delta."));
    QCOMPARE(indentAt(2, 14), 0.0);

    Settings::instance()->setTextAutoIndent(false);
}

/**! @brief With the setting on, a space typed at the end of a paragraph is
 * not underlined while the cursor is after it, but is once the cursor
 * leaves, and stays so when the cursor comes back. Runs of spaces are
 * underlined straight away.
 */
void TestGroupEditor::multiSpaces()
{
    Settings::instance()->setShowMultiSpaces(true);
    QTextBlock block = f->doc()->findBlock(f->textPosition(4, 0));
    auto underlined = [&block]() {
        QList<int> starts;
        for (const QTextLayout::FormatRange &range : block.layout()->formats()) {
            if (range.format.underlineStyle() == QTextCharFormat::SingleUnderline) starts.append(range.start);
        }
        return starts;
    };

    f->enterText(4, 8);
    QTRY_COMPARE(f->focusCursor(), 8);
    f->type(" ");
    QTRY_COMPARE(f->text(4), QStringLiteral("Epsilon. "));
    QCOMPARE(underlined(), QList<int>());

    QTest::keyClick(f->window, Qt::Key_Left);
    QTest::keyClick(f->window, Qt::Key_Left);
    QTRY_COMPARE(underlined(), QList<int>({8}));

    // Moving back to it does not hide it
    QTest::keyClick(f->window, Qt::Key_End);
    QTRY_COMPARE(f->focusCursor(), 9);
    QTest::qWait(20);
    QCOMPARE(underlined(), QList<int>({8}));
    f->type(" x");
    QTRY_COMPARE(f->text(4), QStringLiteral("Epsilon.  x"));
    QCOMPARE(underlined(), QList<int>({8}));

    Settings::instance()->setShowMultiSpaces(false);
    QTRY_COMPARE(underlined(), QList<int>());
}

/**! @brief A misspelled word is not underlined while it is being typed,
 * only once the cursor moves on. Moving back into it keeps the underline.
 */
void TestGroupEditor::typingWord()
{
    SpellChecker *spell = f->project.spellChecker();
    spell->setDictionaryPaths({QStringLiteral(TEST_DATA_DIR)});
    spell->setLanguage("xx_TEST");
    QVERIFY(spell->isLoaded());

    QTextBlock block = f->doc()->findBlock(f->textPosition(4, 0));
    auto marked = [&block]() {
        QStringList words;
        for (const QTextLayout::FormatRange &range : block.layout()->formats()) {
            if (range.format.underlineStyle() == QTextCharFormat::SpellCheckUnderline) {
                words.append(block.text().mid(range.start, range.length));
            }
        }
        return words;
    };

    f->enterText(4, 8);
    QTRY_COMPARE(f->focusCursor(), 8);
    f->type(" helo");
    QTRY_COMPARE(f->text(4), QStringLiteral("Epsilon. helo"));
    QVERIFY(!marked().contains("helo"));

    f->type(" ");
    QTRY_VERIFY(marked().contains("helo"));

    // Moving back into the word to correct it keeps the underline
    QTest::keyClick(f->window, Qt::Key_Left);
    QTest::keyClick(f->window, Qt::Key_Left);
    QTRY_COMPARE(f->focusCursor(), 12);
    QTest::qWait(20);
    QVERIFY(marked().contains("helo"));

    // Until it is edited
    f->type("l");
    QTRY_COMPARE(f->text(4), QStringLiteral("Epsilon. hello "));
    QVERIFY(!marked().contains("hello"));
}

/**! @brief The type menu deletes the document after asking.
 */
void TestGroupEditor::deleteFromTypeMenu()
{
    const QString scene = f->handle(4);
    QQuickItem *label = f->typeLabel(4);
    QObject *menu = f->window->findChild<QObject *>("typeMenu");
    QVERIFY(label && menu);
    QTest::mouseClick(f->window, Qt::LeftButton, Qt::NoModifier, f->pointIn(label));
    QTRY_VERIFY(menu->property("opened").toBool());

    QQuickItem *item = menuItem(menu, "Delete Document…");
    QVERIFY(item && item->isEnabled());
    QTest::mouseClick(f->window, Qt::LeftButton, Qt::NoModifier, f->pointIn(item));
    QObject *dialog = f->window->findChild<QObject *>("deleteDialog");
    QTRY_VERIFY(dialog->property("opened").toBool());
    QCOMPARE(dialog->property("name").toString(), QStringLiteral("2.1 Scene"));
    QMetaObject::invokeMethod(dialog, "accept");
    QTRY_COMPARE(f->rows(), 5);
    QCOMPARE(f->model()->rowOf(scene), -1);
}

/**! @brief After a split at the end of the text, the new document has an
 * empty title and no text. Enter in the title starts its text. The cursor
 * shows in both.
 */
void TestGroupEditor::emptyTextCursor()
{
    f->enterText(4, 8);
    QTRY_COMPARE(f->focusCursor(), 8);
    QTest::keyClick(f->window, Qt::Key_Return, Qt::ControlModifier);
    QTRY_COMPARE(f->rows(), 7);
    QTRY_COMPARE(f->focusPart(), QStringLiteral("title"));
    QCOMPARE(f->text(5), QString());

    auto cursorShown = [this]() {
        QQuickItem *text = f->textEdit();
        const QRectF cursor = text->property("cursorRectangle").toRectF();
        return text->property("cursorVisible").toBool() && cursor.height() > 0 && cursor.left() >= 0;
    };
    QVERIFY(cursorShown());

    QTest::keyClick(f->window, Qt::Key_Return);
    QTRY_COMPARE(f->focusPart(), QStringLiteral("text"));
    QCOMPARE(f->focusHandle(), f->handle(5));
    QCOMPARE(f->text(5), QString());
    QCOMPARE(f->doc()->titleBlock(f->handle(5)).next().length(), 1);
    QVERIFY(cursorShown());
}

/**! @brief Moving down keeps the cursor at the same horizontal position,
 * through short lines, titles, empty text and other documents. Moving the
 * cursor sideways starts a new position.
 */
void TestGroupEditor::goalColumn()
{
    auto cursorX = [this]() {
        QQuickItem *item = f->window->activeFocusItem();
        return item->mapToScene(item->property("cursorRectangle").toRectF().topLeft()).x();
    };
    f->enterText(2, 20);
    QTRY_COMPARE(f->focusCursor(), 20);
    const qreal start = cursorX();

    // Through the chapter title, the empty title and short text of the next
    // scene, and the empty title of the last scene
    for (int i = 0; i < 5; ++i)
        QTest::keyClick(f->window, Qt::Key_Down);
    QTRY_COMPARE(f->focusHandle(), f->handle(5));
    QCOMPARE(f->focusPart(), QStringLiteral("text"));
    QVERIFY(qAbs(cursorX() - start) < 12);

    // And back up again
    for (int i = 0; i < 5; ++i)
        QTest::keyClick(f->window, Qt::Key_Up);
    QTRY_COMPARE(f->focusHandle(), f->handle(2));
    QCOMPARE(f->focusCursor(), 20);

    // A sideways move starts over from the new position
    QTest::keyClick(f->window, Qt::Key_Home);
    QTest::keyClick(f->window, Qt::Key_Up);
    QTRY_COMPARE(f->focusCursor(), 0);
    QTest::keyClick(f->window, Qt::Key_Up);
    QTRY_COMPARE(f->focusPart(), QStringLiteral("title"));
    QTest::keyClick(f->window, Qt::Key_Down);
    QTRY_COMPARE(f->focusPart(), QStringLiteral("text"));
    QCOMPARE(f->focusCursor(), 0);
}

/**! @brief A split is one step in the undo history of the text.
 */
void TestGroupEditor::undoSplit()
{
    const QStringList order = f->order();
    f->enterText(2, 6);
    QTRY_COMPARE(f->focusCursor(), 6);
    QTest::keyClick(f->window, Qt::Key_Return, Qt::ControlModifier);
    QTRY_COMPARE(f->rows(), 7);

    QTest::keyClick(f->window, Qt::Key_Z, Qt::ControlModifier);
    QTRY_COMPARE(f->rows(), 6);
    QCOMPARE(f->order(), order);
    QCOMPARE(f->text(2), QStringLiteral("Alpha beta.\nGamma delta."));

    QTest::keyClick(f->window, Qt::Key_Z, Qt::ControlModifier | Qt::ShiftModifier);
    QTRY_COMPARE(f->rows(), 7);
    QCOMPARE(f->text(3), QStringLiteral("beta.\nGamma delta."));
}

/**! @brief A deleted document comes back with undo, with its handle, text
 * and type.
 */
void TestGroupEditor::undoDelete()
{
    const QStringList order = f->order();
    f->enterText(5, 0);
    QTRY_COMPARE(f->focusHandle(), f->handle(5));
    QVERIFY(f->binder()->deleteItem(f->handle(3)));
    QTRY_COMPARE(f->rows(), 5);
    QCOMPARE(f->model()->rowOf(order.at(3)), -1);

    QTest::keyClick(f->window, Qt::Key_Z, Qt::ControlModifier);
    QTRY_COMPARE(f->rows(), 6);
    QCOMPARE(f->order(), order);
    QCOMPARE(f->value(3, ProjectModel::TitleRole).toString(), QStringLiteral("Two"));
    QCOMPARE(f->value(3, ProjectModel::LevelRole).toInt(), int(ItemLevel::ChapterLevel));
    QCOMPARE(f->text(4), QStringLiteral("Epsilon."));
}

/**! @brief A selection can span documents. Deleting it removes the
 * documents whose titles were in it, and the text left joins the first
 * document.
 */
void TestGroupEditor::selectAcross()
{
    const QStringList order = f->order();
    f->enterText(2, 6);
    QTRY_COMPARE(f->focusCursor(), 6);
    QTextCursor cursor(f->doc());
    const int start = f->textPosition(2, 6);
    const int end = f->textPosition(4, 4);
    f->textEdit()->setProperty("cursorPosition", start);
    QMetaObject::invokeMethod(f->textEdit(), "moveCursorSelection", Q_ARG(int, end));
    QTRY_COMPARE(f->textEdit()->property("selectedText").toString().length(), end - start);

    QTest::keyClick(f->window, Qt::Key_Delete);
    QTRY_COMPARE(f->rows(), 4);
    QCOMPARE(f->text(2), QStringLiteral("Alpha lon."));
    QCOMPARE(f->handle(2), order.at(2));
    QCOMPARE(f->handle(3), order.at(5));

    QTest::keyClick(f->window, Qt::Key_Z, Qt::ControlModifier);
    QTRY_COMPARE(f->rows(), 6);
    QCOMPARE(f->order(), order);
}

/**! @brief The title is typed in the text, and the project follows. Enter
 * moves on to the text, and a title takes no line breaks or tabs.
 */
void TestGroupEditor::typeTitle()
{
    f->enterTitle(2);
    QTRY_COMPARE(f->focusPart(), QStringLiteral("title"));
    f->type("Intro");
    QTRY_COMPARE(f->value(2, ProjectModel::TitleRole).toString(), QStringLiteral("Intro"));
    QCOMPARE(f->listItem(2)->property("name").toString(), QStringLiteral("1.1 Intro"));

    QTest::keyClick(f->window, Qt::Key_Return, Qt::ShiftModifier);
    QTest::keyClick(f->window, Qt::Key_Tab);
    QTest::qWait(20);
    QCOMPARE(f->value(2, ProjectModel::TitleRole).toString(), QStringLiteral("Intro"));
    QCOMPARE(f->rows(), 6);

    QTest::keyClick(f->window, Qt::Key_Return);
    QTRY_COMPARE(f->focusPart(), QStringLiteral("text"));
    QCOMPARE(f->focusCursor(), 0);
    QCOMPARE(f->text(2), QStringLiteral("Alpha beta.\nGamma delta."));

    // Backspace at the start of a titled document only goes to the title
    QTest::keyClick(f->window, Qt::Key_Backspace);
    QTRY_COMPARE(f->focusPart(), QStringLiteral("title"));
    QCOMPARE(f->focusCursor(), 5);
    QCOMPARE(f->rows(), 6);
}

/**! @brief Delete at the end of a text merges the next document if its title
 * is empty, and does nothing if it has a title.
 */
void TestGroupEditor::deleteAtEnd()
{
    f->enterText(2, f->text(2).length());
    QTRY_COMPARE(f->focusHandle(), f->handle(2));
    QTest::keyClick(f->window, Qt::Key_Delete);
    QTest::qWait(20);
    QCOMPARE(f->rows(), 6);
    QCOMPARE(f->value(3, ProjectModel::TitleRole).toString(), QStringLiteral("Two"));

    const QString merged = f->handle(5);
    f->enterText(4, f->text(4).length());
    QTRY_COMPARE(f->focusHandle(), f->handle(4));
    QTest::keyClick(f->window, Qt::Key_Delete);
    QTRY_COMPARE(f->rows(), 5);
    QCOMPARE(f->model()->rowOf(merged), -1);
    QCOMPARE(f->text(4), QStringLiteral("Epsilon.\nZeta eta theta."));
    QCOMPARE(f->focusCursor(), 8);
}

/**! @brief A line break pasted into a title moves the text after it to the
 * text of the document, in the same undo step.
 */
void TestGroupEditor::lineBreakInTitle()
{
    QTextCursor cursor(f->doc());
    cursor.setPosition(f->doc()->titleBlock(f->handle(3)).position() + 3);
    cursor.insertText(" A\nB");
    QTRY_COMPARE(f->text(3), QStringLiteral("B"));
    QCOMPARE(f->value(3, ProjectModel::TitleRole).toString(), QStringLiteral("Two A"));
    QCOMPARE(f->rows(), 6);

    f->doc()->undo();
    QTRY_COMPARE(f->value(3, ProjectModel::TitleRole).toString(), QStringLiteral("Two"));
    QCOMPARE(f->text(3), QString());
}

/**! @brief Deleting from the start of the text into a document leaves the
 * rest of it with a new, empty title, in the same undo step.
 */
void TestGroupEditor::firstTitleRemoved()
{
    const QStringList order = f->order();
    QTextCursor cursor(f->doc());
    cursor.setPosition(f->textPosition(2, 6));
    cursor.setPosition(0, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    QTRY_COMPARE(f->rows(), 4);
    QVERIFY(!order.contains(f->handle(0)));
    QCOMPARE(f->value(0, ProjectModel::TitleRole).toString(), QString());
    QCOMPARE(f->text(0), QStringLiteral("beta.\nGamma delta."));
    QCOMPARE(f->handle(1), order.at(3));

    f->doc()->undo();
    QTRY_COMPARE(f->rows(), 6);
    QCOMPARE(f->order(), order);
}

QTEST_MAIN(TestGroupEditor)
#include "tst_groupeditor.moc"
