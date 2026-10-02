/*
** Collett - Toggle Switch Widget Tests
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

#include "mswitch.h"
#include "theme.h"

#include <QSignalSpy>
#include <QtTest>

using namespace Collett;

class TestMSwitch : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void geometry();
    void setChecked();
    void click();
};

void TestMSwitch::initTestCase()
{
    QCoreApplication::setOrganizationName("CollettTest");
    QCoreApplication::setApplicationName("tst_mswitch");
}

void TestMSwitch::cleanupTestCase()
{
    Theme::destroy();
}

/**! @brief The switch is fixed at twice its height, from the theme or as given.
 */
void TestMSwitch::geometry()
{
    MSwitch themed;
    const int height = Theme::instance()->baseButtonHeight();
    QCOMPARE(themed.sizeHint(), QSize(2 * height, height));
    QCOMPARE(themed.size(), QSize(2 * height, height));

    MSwitch sized(nullptr, 20);
    QCOMPARE(sized.sizeHint(), QSize(40, 20));
    QCOMPARE(sized.minimumSizeHint(), QSize(40, 20));
    QCOMPARE(sized.size(), QSize(40, 20));
    QVERIFY(sized.isCheckable());
    QVERIFY(!sized.isChecked());
}

/**! @brief Setting the state programmatically moves the knob immediately.
 */
void TestMSwitch::setChecked()
{
    MSwitch button(nullptr, 20);
    QCOMPARE(button.offset(), 10);

    button.setChecked(true);
    QVERIFY(button.isChecked());
    QCOMPARE(button.offset(), 30);

    button.setChecked(false);
    QVERIFY(!button.isChecked());
    QCOMPARE(button.offset(), 10);
}

/**! @brief Clicking toggles the state and slides the knob to the new position.
 */
void TestMSwitch::click()
{
    MSwitch button(nullptr, 20);
    button.show();
    QVERIFY(QTest::qWaitForWindowExposed(&button));

    QSignalSpy spy(&button, &QAbstractButton::toggled);
    QTest::mouseClick(&button, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
    QVERIFY(button.isChecked());
    QTRY_COMPARE(button.offset(), 30);

    QTest::mouseClick(&button, Qt::LeftButton);
    QCOMPARE(spy.count(), 2);
    QVERIFY(!button.isChecked());
    QTRY_COMPARE(button.offset(), 10);
}

QTEST_MAIN(TestMSwitch)
#include "tst_mswitch.moc"
