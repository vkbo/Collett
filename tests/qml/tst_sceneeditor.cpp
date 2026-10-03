/*
** Collett - Scene Editor Tests
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

#include <QtTest>

using namespace Collett;

class TestSceneEditor : public QObject
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

private:
    QmlFixture *f = nullptr;

    QPoint pointInText(int row, int position) const;
    QQuickItem *menuItem(QObject *menu, const QString &text) const;
};

/**! @brief The window position of a character in the text of a document.
 */
QPoint TestSceneEditor::pointInText(int row, int position) const
{
    QQuickItem *text = f->scene(row)->findChild<QQuickItem *>("textEdit");
    QRectF rect;
    QMetaObject::invokeMethod(text, "positionToRectangle", Q_RETURN_ARG(QRectF, rect), Q_ARG(int, position));
    return text->mapToScene(rect.center()).toPoint();
}

/**! @brief The item of a menu with a given text.
 */
QQuickItem *TestSceneEditor::menuItem(QObject *menu, const QString &text) const
{
    const int count = menu->property("count").toInt();
    for (int i = 0; i < count; ++i) {
        QQuickItem *item = nullptr;
        QMetaObject::invokeMethod(menu, "itemAt", Q_RETURN_ARG(QQuickItem *, item), Q_ARG(int, i));
        if (item && item->property("text").toString() == text) return item;
    }
    return nullptr;
}

void TestSceneEditor::initTestCase()
{
    initQmlTests();
}

/**! @brief Every test starts from the same project: a title page, then
 * chapter 1 with one scene, and chapter 2 with two scenes.
 */
void TestSceneEditor::init()
{
    f = new QmlFixture();
    QVERIFY(f->create());
    f->addDocument(ItemLevel::ChapterLevel, "One", "");
    f->addDocument(ItemLevel::SceneLevel, "", "Alpha beta.\nGamma delta.");
    f->addDocument(ItemLevel::ChapterLevel, "Two", "");
    f->addDocument(ItemLevel::SceneLevel, "", "Epsilon.");
    f->addDocument(ItemLevel::SceneLevel, "", "Zeta eta theta.");
    QVERIFY(f->load());
}

void TestSceneEditor::cleanup()
{
    delete f;
    f = nullptr;
}

/**! @brief Ctrl+Enter moves the text after the cursor into a new scene, and
 * puts the cursor in its title.
 */
void TestSceneEditor::splitAtCursor()
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
void TestSceneEditor::splitCyclesType()
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
void TestSceneEditor::backspaceMerges()
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
void TestSceneEditor::backspaceRemovesEmpty()
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
void TestSceneEditor::arrowsCrossDocuments()
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
void TestSceneEditor::liveWordCount()
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
void TestSceneEditor::spellingMenu()
{
    SpellChecker *spell = f->project.spellChecker();
    spell->setDictionaryPaths({QStringLiteral(TEST_DATA_DIR)});
    spell->setLanguage("xx_TEST");
    QVERIFY(spell->isLoaded());

    QTextCursor cursor(f->project.openDocument(f->handle(5)));
    cursor.select(QTextCursor::Document);
    cursor.insertText("hello wrld");
    QObject *menu = f->scene(5)->findChild<QObject *>("spellMenu");
    QVERIFY(menu);

    // A correct word has no menu
    QTest::mouseClick(f->window, Qt::RightButton, Qt::NoModifier, pointInText(5, 2));
    QTest::qWait(50);
    QVERIFY(!menu->property("opened").toBool());

    // Choosing a suggestion replaces the word
    QTest::mouseClick(f->window, Qt::RightButton, Qt::NoModifier, pointInText(5, 8));
    QTRY_VERIFY(menu->property("opened").toBool());
    QQuickItem *suggestion = menuItem(menu, "world");
    QVERIFY(suggestion);
    QTest::mouseClick(f->window, Qt::LeftButton, Qt::NoModifier, f->pointIn(suggestion));
    QTRY_COMPARE(f->text(5), QStringLiteral("hello world"));
    QTRY_VERIFY(!menu->property("opened").toBool());

    // Adding a word to the dictionary accepts it from then on
    cursor.select(QTextCursor::Document);
    cursor.insertText("hello Gollum");
    QVERIFY(!spell->checkWord("Gollum"));
    QTest::mouseClick(f->window, Qt::RightButton, Qt::NoModifier, pointInText(5, 8));
    QTRY_VERIFY(menu->property("opened").toBool());
    QQuickItem *add = menuItem(menu, "Add to Dictionary");
    QVERIFY(add);
    QTest::mouseClick(f->window, Qt::LeftButton, Qt::NoModifier, f->pointIn(add));
    QTRY_VERIFY(spell->checkWord("Gollum"));
    QCOMPARE(f->text(5), QStringLiteral("hello Gollum"));
}

QTEST_MAIN(TestSceneEditor)
#include "tst_sceneeditor.moc"
