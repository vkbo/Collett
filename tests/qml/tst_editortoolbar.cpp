/*
** Collett - Editor Tool Bar Tests
** ===============================
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

#include <QQmlProperty>
#include <QTextBlock>
#include <QTextCursor>
#include <QtTest>

using namespace Collett;

class TestEditorToolBar : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void formatWord();
    void formatSelection();
    void formatTyping();
    void alignAndIndent();
    void headingStyles();
    void enterAfterHeading();
    void onlyInText();

private:
    QmlFixture *f = nullptr;

    QQuickItem *button(const QString &name) const { return f->item(name); }
    QTextCharFormat formatAt(int row, int position);
    QTextBlockFormat blockAt(int row, int position);
};

/**! @brief The character format after a position in the text of a document.
 */
QTextCharFormat TestEditorToolBar::formatAt(int row, int position)
{
    QTextCursor cursor(f->project.openDocument(f->handle(row)));
    cursor.setPosition(position + 1);
    return cursor.charFormat();
}

QTextBlockFormat TestEditorToolBar::blockAt(int row, int position)
{
    return f->project.openDocument(f->handle(row))->findBlock(position).blockFormat();
}

void TestEditorToolBar::initTestCase()
{
    initQmlTests();
}

/**! @brief Every test starts from a chapter with one scene of two paragraphs.
 */
void TestEditorToolBar::init()
{
    f = new QmlFixture();
    QVERIFY(f->create());
    f->addDocument(ItemLevel::ChapterLevel, "One", "");
    f->addDocument(ItemLevel::SceneLevel, "", "Alpha beta.\nGamma delta.");
    QVERIFY(f->load());
}

void TestEditorToolBar::cleanup()
{
    delete f;
    f = nullptr;
}

/**! @brief With no selection, a format goes on the word at the cursor, and
 * the button follows the format at the cursor. The cursor stays in the text.
 */
void TestEditorToolBar::formatWord()
{
    f->enterText(2, 2);
    QTRY_COMPARE(f->focusPart(), QStringLiteral("text"));
    QVERIFY(button("boldButton")->isEnabled());
    QVERIFY(!button("boldButton")->property("checked").toBool());

    QTest::mouseClick(f->window, Qt::LeftButton, Qt::NoModifier, f->pointIn(button("boldButton")));
    QTRY_COMPARE(formatAt(2, 0).fontWeight(), int(QFont::Bold));
    QCOMPARE(formatAt(2, 4).fontWeight(), int(QFont::Bold));
    QCOMPARE(formatAt(2, 6).fontWeight(), int(QFont::Normal));
    QTRY_VERIFY(button("boldButton")->property("checked").toBool());
    QCOMPARE(f->focusPart(), QStringLiteral("text"));

    QTest::keyClick(f->window, Qt::Key_I, Qt::ControlModifier);
    QTRY_VERIFY(formatAt(2, 0).fontItalic());
    QTRY_VERIFY(button("italicButton")->property("checked").toBool());

    // Moving to plain text clears the buttons
    f->enterText(2, 8);
    QTRY_VERIFY(!button("boldButton")->property("checked").toBool());
    QVERIFY(!button("italicButton")->property("checked").toBool());

    // Superscript and subscript replace each other
    QTest::keyClick(f->window, Qt::Key_Equal, Qt::ControlModifier | Qt::ShiftModifier);
    QTRY_COMPARE(formatAt(2, 7).verticalAlignment(), QTextCharFormat::AlignSuperScript);
    QTest::keyClick(f->window, Qt::Key_Equal, Qt::ControlModifier);
    QTRY_COMPARE(formatAt(2, 7).verticalAlignment(), QTextCharFormat::AlignSubScript);
    QTRY_VERIFY(button("subscriptButton")->property("checked").toBool());
    QVERIFY(!button("superscriptButton")->property("checked").toBool());
}

/**! @brief A format goes on the whole selection, and toggles off again.
 */
void TestEditorToolBar::formatSelection()
{
    f->enterText(2, 0);
    QTRY_COMPARE(f->focusPart(), QStringLiteral("text"));
    for (int i = 0; i < 10; ++i)
        QTest::keyClick(f->window, Qt::Key_Right, Qt::ShiftModifier);

    QTest::keyClick(f->window, Qt::Key_U, Qt::ControlModifier);
    QTRY_VERIFY(formatAt(2, 0).fontUnderline());
    QVERIFY(formatAt(2, 9).fontUnderline());
    QVERIFY(!formatAt(2, 10).fontUnderline());

    QTest::keyClick(f->window, Qt::Key_D, Qt::ControlModifier);
    QTRY_VERIFY(formatAt(2, 5).fontStrikeOut());
    QTRY_VERIFY(button("strikeOutButton")->property("checked").toBool());

    QTest::keyClick(f->window, Qt::Key_U, Qt::ControlModifier);
    QTRY_VERIFY(!formatAt(2, 0).fontUnderline());
    QVERIFY(formatAt(2, 0).fontStrikeOut());
}

/**! @brief A format set where there is no word applies to the text typed
 * next, and is undone with it.
 */
void TestEditorToolBar::formatTyping()
{
    f->enterText(2, 11);
    QTRY_COMPARE(f->focusPart(), QStringLiteral("text"));
    f->type(" ");

    QTest::keyClick(f->window, Qt::Key_B, Qt::ControlModifier);
    QTRY_VERIFY(button("boldButton")->property("checked").toBool());
    f->type("xy");
    QTRY_COMPARE(f->text(2), QStringLiteral("Alpha beta. xy\nGamma delta."));
    QCOMPARE(formatAt(2, 10).fontWeight(), int(QFont::Normal));
    QCOMPARE(formatAt(2, 11).fontWeight(), int(QFont::Normal));
    QCOMPARE(formatAt(2, 12).fontWeight(), int(QFont::Bold));
    QCOMPARE(formatAt(2, 13).fontWeight(), int(QFont::Bold));
    QVERIFY(button("boldButton")->property("checked").toBool());

    // A held back format is dropped when the cursor moves away
    f->type(" ");
    QTest::keyClick(f->window, Qt::Key_B, Qt::ControlModifier);
    QTRY_VERIFY(!button("boldButton")->property("checked").toBool());
    QTest::keyClick(f->window, Qt::Key_I, Qt::ControlModifier);
    QTRY_VERIFY(button("italicButton")->property("checked").toBool());
    QTest::keyClick(f->window, Qt::Key_Left);
    QTRY_VERIFY(!button("italicButton")->property("checked").toBool());
}

/**! @brief Alignment and indent apply to the paragraph at the cursor.
 */
void TestEditorToolBar::alignAndIndent()
{
    f->enterText(2, 14);
    QTRY_COMPARE(f->focusPart(), QStringLiteral("text"));
    QTRY_VERIFY(button("alignLeftButton")->property("checked").toBool());

    QTest::keyClick(f->window, Qt::Key_E, Qt::ControlModifier);
    QTRY_COMPARE(blockAt(2, 14).alignment(), Qt::AlignHCenter);
    QCOMPARE(blockAt(2, 0).alignment(), Qt::AlignLeft);
    QTRY_VERIFY(button("alignCenterButton")->property("checked").toBool());
    QVERIFY(!button("alignLeftButton")->property("checked").toBool());

    QTest::mouseClick(f->window, Qt::LeftButton, Qt::NoModifier, f->pointIn(button("alignJustifyButton")));
    QTRY_COMPARE(blockAt(2, 14).alignment(), Qt::AlignJustify);
    QTRY_VERIFY(button("alignJustifyButton")->property("checked").toBool());

    QTest::keyClick(f->window, Qt::Key_M, Qt::ControlModifier);
    QTest::keyClick(f->window, Qt::Key_M, Qt::ControlModifier);
    QTRY_COMPARE(blockAt(2, 14).indent(), 2);
    QTest::keyClick(f->window, Qt::Key_M, Qt::ControlModifier | Qt::ShiftModifier);
    QTRY_COMPARE(blockAt(2, 14).indent(), 1);
    QCOMPARE(blockAt(2, 0).indent(), 0);
}

/**! @brief The style menu turns the paragraph at the cursor into a heading
 * and back, keeping its alignment and the italic text in it.
 */
void TestEditorToolBar::headingStyles()
{
    f->enterText(2, 14);
    QTRY_COMPARE(f->focusPart(), QStringLiteral("text"));
    QTest::keyClick(f->window, Qt::Key_E, Qt::ControlModifier);
    QTest::keyClick(f->window, Qt::Key_I, Qt::ControlModifier);
    QTRY_VERIFY(formatAt(2, 14).fontItalic());
    QCOMPARE(QQmlProperty::read(button("styleButton"), "icon.source").toUrl(), QUrl("image://icons/style_normal"));

    QObject *menu = f->window->findChild<QObject *>("styleMenu");
    QVERIFY(menu);
    QTest::mouseClick(f->window, Qt::LeftButton, Qt::NoModifier, f->pointIn(button("styleButton")));
    QTRY_VERIFY(menu->property("opened").toBool());
    QQuickItem *item = f->window->findChild<QQuickItem *>("styleItem2");
    QVERIFY(item);
    QTest::mouseClick(f->window, Qt::LeftButton, Qt::NoModifier, f->pointIn(item));
    QTRY_VERIFY(!menu->property("visible").toBool());

    const Settings::TextFormat format = Settings::instance()->textFormat();
    QTRY_COMPARE(blockAt(2, 14).headingLevel(), 2);
    QCOMPARE(blockAt(2, 0).headingLevel(), 0);
    QCOMPARE(blockAt(2, 14).alignment(), Qt::AlignHCenter);
    QCOMPARE(formatAt(2, 14).fontPointSize(), format.charHeader2.fontPointSize());
    QVERIFY(formatAt(2, 14).fontItalic());
    QTRY_COMPARE(QQmlProperty::read(button("styleButton"), "icon.source").toUrl(), QUrl("image://icons/style_h2"));
    QTRY_COMPARE(f->focusPart(), QStringLiteral("text"));

    QTest::mouseClick(f->window, Qt::LeftButton, Qt::NoModifier, f->pointIn(button("styleButton")));
    QTRY_VERIFY(menu->property("opened").toBool());
    item = f->window->findChild<QQuickItem *>("styleItem0");
    QTest::mouseClick(f->window, Qt::LeftButton, Qt::NoModifier, f->pointIn(item));
    QTRY_COMPARE(blockAt(2, 14).headingLevel(), 0);
    QCOMPARE(formatAt(2, 14).fontPointSize(), format.charParagraph.fontPointSize());
    QCOMPARE(formatAt(2, 14).fontWeight(), int(QFont::Normal));
    QVERIFY(formatAt(2, 14).fontItalic());
}

/**! @brief Enter at the end of a heading starts a plain paragraph, while
 * Enter inside a heading splits it into two headings.
 */
void TestEditorToolBar::enterAfterHeading()
{
    const Settings::TextFormat format = Settings::instance()->textFormat();
    f->enterText(2, 3);
    QTRY_COMPARE(f->focusPart(), QStringLiteral("text"));
    QQuickItem *binderOwner = f->scene(2);
    QObject *binder = binderOwner->property("textBinder").value<QObject *>();
    QVERIFY(binder);
    binder->setProperty("headingLevel", 1);
    QTRY_COMPARE(blockAt(2, 0).headingLevel(), 1);

    // At the end
    f->enterText(2, 11);
    QTRY_COMPARE(f->focusCursor(), 11);
    QTest::keyClick(f->window, Qt::Key_Return);
    f->type("New");
    QTRY_COMPARE(f->text(2), QStringLiteral("Alpha beta.\nNew\nGamma delta."));
    QCOMPARE(blockAt(2, 0).headingLevel(), 1);
    QCOMPARE(blockAt(2, 12).headingLevel(), 0);
    QCOMPARE(formatAt(2, 12).fontPointSize(), format.charParagraph.fontPointSize());
    QCOMPARE(formatAt(2, 12).fontWeight(), int(QFont::Normal));
    QTRY_COMPARE(binder->property("headingLevel").toInt(), 0);

    // In the middle
    f->enterText(2, 6);
    QTRY_COMPARE(f->focusCursor(), 6);
    QTest::keyClick(f->window, Qt::Key_Return);
    QTRY_COMPARE(f->text(2), QStringLiteral("Alpha \nbeta.\nNew\nGamma delta."));
    QCOMPARE(blockAt(2, 0).headingLevel(), 1);
    QCOMPARE(blockAt(2, 7).headingLevel(), 1);
}

/**! @brief The tool bar only works while the cursor is in the text.
 */
void TestEditorToolBar::onlyInText()
{
    f->enterText(2, 2);
    QTRY_VERIFY(button("boldButton")->isEnabled());

    QMetaObject::invokeMethod(f->scene(2), "enterTitleAt", Q_ARG(int, 0));
    QTRY_COMPARE(f->focusPart(), QStringLiteral("title"));
    QTRY_VERIFY(!button("boldButton")->isEnabled());
    QTest::keyClick(f->window, Qt::Key_B, Qt::ControlModifier);
    QTest::qWait(50);
    QCOMPARE(formatAt(2, 0).fontWeight(), int(QFont::Normal));
}

QTEST_MAIN(TestEditorToolBar)
#include "tst_editortoolbar.moc"
