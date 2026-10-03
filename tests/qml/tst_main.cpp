/*
** Collett - Main Window Tests
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

#include "qmlfixture.h"
#include "settings.h"

#include <QQuickStyle>
#include <QtTest>

using namespace Collett;

class TestMain : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void styleFromConfig();
    void levelColours();
    void themeButton();
    void itemOpensDocument();
    void dragScene();
    void dragFoldedChapter();
    void dragBelowLast();
    void newProject();

private:
    QmlFixture *f = nullptr;

    void build();
    void drag(int from, int onto, qreal fraction);
    QPoint itemPoint(int row, qreal fraction) const;
};

void TestMain::initTestCase()
{
    initQmlTests();
}

void TestMain::init()
{
    Settings::instance()->setThemeMode(Settings::AutoTheme);
    f = new QmlFixture();
}

void TestMain::cleanup()
{
    delete f;
    f = nullptr;
}

/**! @brief A title page, then chapter 1 with one scene, and chapter 2 with
 * two scenes.
 */
void TestMain::build()
{
    QVERIFY(f->create());
    f->addDocument(ItemLevel::ChapterLevel, "One", "");
    f->addDocument(ItemLevel::SceneLevel, "", "Alpha beta.");
    f->addDocument(ItemLevel::ChapterLevel, "Two", "");
    f->addDocument(ItemLevel::SceneLevel, "", "Gamma.");
    f->addDocument(ItemLevel::SceneLevel, "", "Delta.");
    QVERIFY(f->load());
}

/**! @brief A point a fraction down the visible item of a row, below any
 * drop gap that is open above it.
 */
QPoint TestMain::itemPoint(int row, qreal fraction) const
{
    QQuickItem *item = f->listItem(row);
    const qreal y = item->property("dropGap").toReal() + item->property("itemHeight").toReal() * fraction;
    return item->mapToScene(QPointF(item->width() / 2, y)).toPoint();
}

/**! @brief Drag an item with the mouse onto a point a fraction down another
 * item, in small steps, like a real drag. The items move aside while
 * dragging, so the target point is taken again on every step, and the last
 * steps wait on it for the items to settle. Each step also waits for a frame,
 * as the move delay of QTest only sets the event time.
 */
void TestMain::drag(int from, int onto, qreal fraction)
{
    const QPoint start = itemPoint(from, 0.5);
    QTest::mousePress(f->window, Qt::LeftButton, Qt::NoModifier, start);
    const int steps = 20;
    QPoint at = start;
    for (int i = 1; i <= steps + 10; ++i) {
        const QPoint end = itemPoint(onto, fraction);
        at = start + (end - start) * qMin(i, steps) / steps;
        QTest::mouseMove(f->window, at);
        QTest::qWait(16);
    }
    QTest::mouseRelease(f->window, Qt::LeftButton, Qt::NoModifier, at);
}

/**! @brief The style comes from the controls config in the resources.
 */
void TestMain::styleFromConfig()
{
    build();
    QCOMPARE(QQuickStyle::name(), QStringLiteral("Material"));
}

/**! @brief The level colours come from the Material palette, in the darker
 * shades used on a light background.
 */
void TestMain::levelColours()
{
    build();
    QObject *theme = f->engine()->singletonInstance<QObject *>("Collett", "Theme");
    QVERIFY(theme);
    QVERIFY(!theme->property("dark").toBool());
    QCOMPARE(theme->property("partitionColor").value<QColor>(), QColor("#388e3c"));
    QCOMPARE(theme->property("chapterColor").value<QColor>(), QColor("#d32f2f"));
    QCOMPARE(theme->property("sceneColor").value<QColor>(), QColor("#1976d2"));
    QCOMPARE(theme->property("pageColor").value<QColor>(), QColor("#757575"));
}

/**! @brief The theme button cycles from following the system, to light, to
 * dark, and back. The parts drawn by the app follow along, not only the
 * controls, and so does the preferences dialog.
 */
void TestMain::themeButton()
{
    build();
    Settings *settings = Settings::instance();
    QObject *theme = f->engine()->singletonInstance<QObject *>("Collett", "Theme");
    QQuickItem *button = f->item("themeButton");
    QVERIFY(theme && button);
    QCOMPARE(settings->themeMode(), Settings::AutoTheme);

    // The editor and side panel backgrounds, and the text of a scene
    QQuickItem *editor = f->item("editorView")->parentItem();
    QQuickItem *panel = f->item("projectList")->parentItem();
    QQuickItem *text = f->scene(2)->findChild<QQuickItem *>("textEdit");
    QQuickWindow *dialog = f->window->findChild<QQuickWindow *>("preferencesDialog");
    QVERIFY(editor && panel && text && dialog);
    auto lightness = [](QObject *object, const char *name) { return object->property(name).value<QColor>().lightness(); };
    QVERIFY(lightness(editor, "color") > 128);
    QVERIFY(lightness(text, "color") < 128);

    QTest::mouseClick(f->window, Qt::LeftButton, Qt::NoModifier, f->pointIn(button));
    QTRY_COMPARE(settings->themeMode(), Settings::LightTheme);
    QVERIFY(!theme->property("dark").toBool());

    QTest::mouseClick(f->window, Qt::LeftButton, Qt::NoModifier, f->pointIn(button));
    QTRY_COMPARE(settings->themeMode(), Settings::DarkTheme);
    QTRY_VERIFY(theme->property("dark").toBool());
    QCOMPARE(theme->property("chapterColor").value<QColor>(), QColor("#e57373"));
    QVERIFY(f->window->color().lightness() < 128);
    QTRY_VERIFY(lightness(editor, "color") < 128);
    QVERIFY(lightness(panel, "color") < 128);
    QVERIFY(lightness(text, "color") > 128);
    QVERIFY(dialog->color().lightness() < 128);

    QTest::mouseClick(f->window, Qt::LeftButton, Qt::NoModifier, f->pointIn(button));
    QTRY_COMPARE(settings->themeMode(), Settings::AutoTheme);
    QTRY_VERIFY(!theme->property("dark").toBool());
    QTRY_VERIFY(lightness(editor, "color") > 128);
}

/**! @brief Clicking an item puts the cursor in its document.
 */
void TestMain::itemOpensDocument()
{
    build();
    QTest::mouseClick(f->window, Qt::LeftButton, Qt::NoModifier, f->pointIn(f->listItem(4)));
    QTRY_COMPARE(f->focusHandle(), f->handle(4));
    QCOMPARE(f->focusPart(), QStringLiteral("text"));
    QCOMPARE(f->window->property("focusHandle").toString(), f->handle(4));
}

/**! @brief Dropping an item on the lower half of another puts it after that
 * one.
 */
void TestMain::dragScene()
{
    build();
    QStringList expected = f->order();
    expected.move(4, 5);

    drag(4, 5, 0.75);
    QTRY_COMPARE(f->order(), expected);
}

/**! @brief A folded chapter is dragged together with its scenes, and stays
 * folded.
 */
void TestMain::dragFoldedChapter()
{
    build();
    const QStringList before = f->order();
    f->model()->toggleExpanded(3);
    QTRY_COMPARE(f->listItem(4)->height(), 0.0);

    drag(3, 1, 0.25);
    const QStringList expected = {before[0], before[3], before[4], before[5], before[1], before[2]};
    QTRY_COMPARE(f->order(), expected);
    QCOMPARE(f->value(1, ProjectModel::ExpandedRole).toBool(), false);
}

/**! @brief Dropping an item in the empty space below the last item puts it
 * last.
 */
void TestMain::dragBelowLast()
{
    build();
    QStringList expected = f->order();
    expected.move(2, 5);

    const QPoint start = itemPoint(2, 0.5);
    QTest::mousePress(f->window, Qt::LeftButton, Qt::NoModifier, start);
    QPoint at = start;
    for (int i = 1; i <= 30; ++i) {
        const QPoint end = itemPoint(5, 1.0) + QPoint(0, 60);
        at = start + (end - start) * qMin(i, 20) / 20;
        QTest::mouseMove(f->window, at);
        QTest::qWait(16);
    }
    QTest::mouseRelease(f->window, Qt::LeftButton, Qt::NoModifier, at);
    QTRY_COMPARE(f->order(), expected);
}

/**! @brief Without a project, the window shows the new project form, and
 * the project it creates opens at its title page.
 */
void TestMain::newProject()
{
    QTemporaryDir location;
    QVERIFY(location.isValid());
    QVERIFY(f->load());
    QVERIFY(!f->project.isValid());

    QQuickItem *name = f->item("nameField");
    QQuickItem *folder = f->item("locationField");
    QQuickItem *create = f->item("createButton");
    QVERIFY(name && folder && create);
    QTRY_VERIFY(name->isVisible());
    QVERIFY(!create->isEnabled());

    QTest::mouseClick(f->window, Qt::LeftButton, Qt::NoModifier, f->pointIn(name));
    f->type("My Novel");
    folder->setProperty("text", location.path());
    QTRY_VERIFY(create->isEnabled());
    QTest::mouseClick(f->window, Qt::LeftButton, Qt::NoModifier, f->pointIn(create));

    QTRY_VERIFY(f->project.isValid());
    QCOMPARE(f->rows(), 1);
    QCOMPARE(f->text(0), QStringLiteral("My Novel"));
    QVERIFY(QFileInfo::exists(location.filePath("My Novel/CollettProject.collett")));
    QTRY_COMPARE(f->focusHandle(), f->handle(0));
}

QTEST_MAIN(TestMain)
#include "tst_main.moc"
