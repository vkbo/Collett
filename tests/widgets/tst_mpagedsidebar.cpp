/*
** Collett - Paged Side Bar Widget Tests
** =====================================
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

#include "mpagedsidebar.h"

#include <QSignalSpy>
#include <QtTest>

using namespace Collett;

class TestMPagedSideBar : public QObject
{
    Q_OBJECT

private slots:
    void buttons();
    void click();
};

/**! @brief Buttons are looked up by id, and selection is exclusive.
 */
void TestMPagedSideBar::buttons()
{
    MPagedSideBar sideBar;
    sideBar.addLabel("Section");
    sideBar.addButton("First", 0);
    sideBar.addButton("Second", 1);

    QVERIFY(sideBar.button(0) != nullptr);
    QVERIFY(sideBar.button(1) != nullptr);
    QVERIFY(sideBar.button(2) == nullptr);
    QCOMPARE(sideBar.button(1)->text(), QStringLiteral("Second"));
    QCOMPARE(sideBar.selected(), -1);

    sideBar.setSelected(1);
    QCOMPARE(sideBar.selected(), 1);
    QVERIFY(!sideBar.button(0)->isChecked());
    QVERIFY(sideBar.button(1)->isChecked());

    sideBar.setSelected(0);
    QCOMPARE(sideBar.selected(), 0);
    QVERIFY(sideBar.button(0)->isChecked());
    QVERIFY(!sideBar.button(1)->isChecked());

    // Unknown ids are ignored
    sideBar.setSelected(5);
    QCOMPARE(sideBar.selected(), 0);
}

/**! @brief Clicking a button emits its id, but programmatic selection does not.
 */
void TestMPagedSideBar::click()
{
    MPagedSideBar sideBar;
    sideBar.addButton("First", 0);
    sideBar.addButton("Second", 1);
    sideBar.show();
    QVERIFY(QTest::qWaitForWindowExposed(&sideBar));

    QSignalSpy spy(&sideBar, &MPagedSideBar::buttonClicked);

    sideBar.setSelected(0);
    QCOMPARE(spy.count(), 0);

    QTest::mouseClick(sideBar.button(1), Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toInt(), 1);
    QCOMPARE(sideBar.selected(), 1);
}

QTEST_MAIN(TestMPagedSideBar)
#include "tst_mpagedsidebar.moc"
