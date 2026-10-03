/*
** Collett - Preferences Dialog Tests
** ==================================
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

#include <QFontDatabase>
#include <QtTest>

using namespace Collett;

class TestPreferences : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void opensFromButton();
    void sideBarFollowsScrolling();
    void saveAndCancel();
    void pickFont();

private:
    QmlFixture *f = nullptr;
    QQuickWindow *dialog = nullptr;

    void openDialog();
    QQuickItem *dialogItem(const QString &name) const;
    QQuickItem *sideBarButton(const QString &text) const;
    QQuickItem *findItem(const std::function<bool(QQuickItem *)> &match) const;
    void click(QQuickItem *item) const;
};

void TestPreferences::initTestCase()
{
    initQmlTests();
}

void TestPreferences::init()
{
    f = new QmlFixture();
    QVERIFY(f->create());
    f->addDocument(ItemLevel::ChapterLevel, "One", "");
    QVERIFY(f->load());
    dialog = f->window->findChild<QQuickWindow *>("preferencesDialog");
    QVERIFY(dialog);
}

void TestPreferences::cleanup()
{
    delete f;
    f = nullptr;
    dialog = nullptr;
}

/**! @brief Open the dialog with the button under the project list.
 */
void TestPreferences::openDialog()
{
    QQuickItem *button = f->item("preferencesButton");
    QVERIFY(button);
    QTest::mouseClick(f->window, Qt::LeftButton, Qt::NoModifier, f->pointIn(button));
    QTRY_VERIFY(dialog->isVisible());
    QVERIFY(QTest::qWaitForWindowExposed(dialog));
}

QQuickItem *TestPreferences::dialogItem(const QString &name) const
{
    return dialog->findChild<QQuickItem *>(name);
}

/**! @brief The first item in the dialog that matches. List delegates are
 * only children in the item tree, so the tree is searched.
 */
QQuickItem *TestPreferences::findItem(const std::function<bool(QQuickItem *)> &match) const
{
    QList<QQuickItem *> items = {dialog->contentItem()};
    while (!items.isEmpty()) {
        QQuickItem *item = items.takeFirst();
        if (item->isVisible() && match(item)) return item;
        items.append(item->childItems());
    }
    return nullptr;
}

QQuickItem *TestPreferences::sideBarButton(const QString &text) const
{
    return findItem([&text](QQuickItem *item) {
        return item->objectName() == "sideBarButton" && item->property("text").toString() == text;
    });
}

void TestPreferences::click(QQuickItem *item) const
{
    QTest::mouseClick(dialog, Qt::LeftButton, Qt::NoModifier, item->mapToScene(QPointF(item->width() / 2, item->height() / 2)).toPoint());
}

/**! @brief The button under the project list opens the dialog.
 */
void TestPreferences::opensFromButton()
{
    QVERIFY(!dialog->isVisible());
    openDialog();
    QVERIFY(dialog->isVisible());
    QCOMPARE(dialog->title(), QStringLiteral("Preferences"));
}

/**! @brief Clicking the side bar scrolls to a section, and scrolling the
 * page moves the side bar selection along.
 */
void TestPreferences::sideBarFollowsScrolling()
{
    openDialog();
    QQuickItem *flow = dialogItem("preferencesFlow");
    QQuickItem *general = sideBarButton("General");
    QQuickItem *spelling = sideBarButton("Spell Checking");
    QVERIFY(flow && general && spelling);
    QVERIFY(general->property("highlighted").toBool());

    QTest::mouseClick(dialog, Qt::LeftButton, Qt::NoModifier, spelling->mapToScene(QPointF(spelling->width() / 2, spelling->height() / 2)).toPoint());
    QTRY_VERIFY(flow->property("contentY").toReal() > 0.0);
    QTRY_VERIFY(!flow->property("moving").toBool());
    QTest::qWait(400);
    QVERIFY(spelling->property("highlighted").toBool());
    QVERIFY(!general->property("highlighted").toBool());

    flow->setProperty("contentY", 0.0);
    QTRY_VERIFY(general->property("highlighted").toBool());
    QVERIFY(!spelling->property("highlighted").toBool());
}

/**! @brief Save keeps the changes, and Cancel drops them.
 */
void TestPreferences::saveAndCancel()
{
    Settings *settings = Settings::instance();
    settings->setEditorAutoSave(30);

    openDialog();
    QQuickItem *autoSave = dialogItem("autoSave");
    QVERIFY(autoSave);
    QCOMPARE(autoSave->property("value").toInt(), 30);

    autoSave->setProperty("value", 45);
    QTest::keyClick(dialog, Qt::Key_Escape);
    QTRY_VERIFY(!dialog->isVisible());
    QCOMPARE(settings->editorAutoSave(), 30);

    openDialog();
    QCOMPARE(autoSave->property("value").toInt(), 30);
    autoSave->setProperty("value", 45);
    QQuickItem *save = dialogItem("saveButton");
    QVERIFY(save);
    QTest::mouseClick(dialog, Qt::LeftButton, Qt::NoModifier, save->mapToScene(QPointF(save->width() / 2, save->height() / 2)).toPoint());
    QTRY_VERIFY(!dialog->isVisible());
    QCOMPARE(settings->editorAutoSave(), 45);
}

/**! @brief The font row opens the font page, where a family is found by
 * searching and picked. Back returns to the settings, and Save keeps it.
 */
void TestPreferences::pickFont()
{
    Settings *settings = Settings::instance();
    QString family;
    for (const QString &name : QFontDatabase::families()) {
        bool plain = true;
        for (const QChar c : name)
            plain = plain && c.unicode() < 128;
        if (plain && name != settings->textFont().family() && !QFontDatabase::styles(name).isEmpty()) {
            family = name;
            break;
        }
    }
    QVERIFY(!family.isEmpty());

    openDialog();
    click(sideBarButton("Fonts"));
    QQuickItem *row = dialogItem("textFontRow");
    QVERIFY(row);
    QTRY_VERIFY(!dialogItem("preferencesFlow")->property("moving").toBool());
    QTest::qWait(300);
    click(row);
    QTRY_VERIFY(findItem([](QQuickItem *item) { return item->objectName() == "fontPage"; }));
    QQuickItem *stack = dialogItem("preferencesStack");
    QTRY_VERIFY(!stack->property("busy").toBool());
    QQuickItem *search = dialogItem("fontSearch");
    QVERIFY(search);
    QTRY_VERIFY(search->hasActiveFocus());

    for (const QChar c : family)
        QTest::keyClick(dialog, c.toLatin1());
    QQuickItem *list = dialogItem("fontFamilies");
    QQuickItem *entry = nullptr;
    QTRY_VERIFY((entry = findItem([&](QQuickItem *item) {
                     return item->parentItem() && item->parentItem()->parentItem() == list && item->property("text").toString() == family;
                 })));
    click(entry);
    QTRY_COMPARE(dialogItem("fontPreview")->property("font").value<QFont>().family(), family);

    click(dialogItem("fontPageBack"));
    QTRY_VERIFY(!stack->property("busy").toBool());
    QCOMPARE(stack->property("depth").toInt(), 1);
    QTRY_VERIFY(row->property("value").toString().contains(family));

    click(dialogItem("saveButton"));
    QTRY_VERIFY(!dialog->isVisible());
    QCOMPARE(settings->textFont().family(), family);
}

QTEST_MAIN(TestPreferences)
#include "tst_preferences.moc"
