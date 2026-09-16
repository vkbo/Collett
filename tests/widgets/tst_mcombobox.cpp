/*
** Collett - Modified QComboBox Tests
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

#include "mcombobox.h"

#include <QPointF>
#include <QWheelEvent>
#include <QtTest>

using namespace Collett;

class TestMComboBox : public QObject
{
    Q_OBJECT

private slots:
    void currentData();
    void wheel();
};

/**! @brief Selecting by data falls back to the default when the data is missing.
 */
void TestMComboBox::currentData()
{
    MComboBox box;
    box.addItem("One", "one");
    box.addItem("Two", "two");
    box.addItem("Three", "three");

    box.setCurrentData("two", "one");
    QCOMPARE(box.currentData().toString(), QStringLiteral("two"));

    box.setCurrentData("missing", "three");
    QCOMPARE(box.currentData().toString(), QStringLiteral("three"));
}

/**! @brief The wheel only changes the value while the box has focus.
 */
void TestMComboBox::wheel()
{
    MComboBox box;
    box.addItem("One");
    box.addItem("Two");
    box.show();
    QVERIFY(QTest::qWaitForWindowExposed(&box));
    box.setCurrentIndex(0);

    const QPointF pos(box.width() / 2.0, box.height() / 2.0);
    QWheelEvent unfocused(pos, box.mapToGlobal(pos.toPoint()), QPoint(), QPoint(0, -120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    box.clearFocus();
    QVERIFY(!box.hasFocus());
    QApplication::sendEvent(&box, &unfocused);
    QVERIFY(!unfocused.isAccepted());
    QCOMPARE(box.currentIndex(), 0);

    box.setFocus();
    QTRY_VERIFY(box.hasFocus());
    QWheelEvent focused(pos, box.mapToGlobal(pos.toPoint()), QPoint(), QPoint(0, -120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(&box, &focused);
    QCOMPARE(box.currentIndex(), 1);
}

QTEST_MAIN(TestMComboBox)
#include "tst_mcombobox.moc"
