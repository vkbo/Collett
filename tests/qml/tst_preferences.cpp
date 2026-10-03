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

private:
    QmlFixture *f = nullptr;
    QQuickWindow *dialog = nullptr;

    void openDialog();
    QQuickItem *dialogItem(const QString &name) const;
    QQuickItem *sideBarButton(const QString &text) const;
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

/**! @brief The side bar entry with a given text. List delegates are only
 * children in the item tree, so the tree is searched.
 */
QQuickItem *TestPreferences::sideBarButton(const QString &text) const
{
    QList<QQuickItem *> items = {dialog->contentItem()};
    while (!items.isEmpty()) {
        QQuickItem *item = items.takeFirst();
        if (item->objectName() == "sideBarButton" && item->property("text").toString() == text) return item;
        items.append(item->childItems());
    }
    return nullptr;
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

QTEST_MAIN(TestPreferences)
#include "tst_preferences.moc"
