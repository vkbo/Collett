/*
** Collett - Scrollable Form Layout Tests
** ======================================
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

#include "scrollableform.h"

#include <QCheckBox>
#include <QPushButton>
#include <QtTest>

using namespace Collett;

class TestScrollableForm : public QObject
{
    Q_OBJECT

private slots:
    void groups();
    void rowWidget();
    void rowButton();
};

/**! @brief Rows go into the latest group, and an unlabelled group is created on demand.
 */
void TestScrollableForm::groups()
{
    MScrollableForm form;
    QCOMPARE(form.groupCount(), 0);
    QCOMPARE(form.rowCount(), 0);

    form.addRow("Orphan", new QCheckBox());
    QCOMPARE(form.groupCount(), 1);
    QCOMPARE(form.rowCount(), 1);

    form.addGroupLabel("First");
    form.addRow("One", new QCheckBox(), "Help");
    form.addRow("Two", new QCheckBox());
    QCOMPARE(form.groupCount(), 2);
    QCOMPARE(form.rowCount(), 3);

    form.addGroupLabel("Second");
    form.addRow("Three", new QCheckBox());
    form.finalise();
    QCOMPARE(form.groupCount(), 3);
    QCOMPARE(form.rowCount(), 4);
}

/**! @brief The row takes ownership of the widget and labels it for accessibility.
 */
void TestScrollableForm::rowWidget()
{
    MScrollableForm form;
    QCheckBox *box = new QCheckBox();
    form.addGroupLabel("Group");
    form.addRow("Setting", box, "Explanation");

    QVERIFY(box->parent() != nullptr);
    QVERIFY(form.isAncestorOf(box));
    QCOMPARE(box->accessibleName(), QStringLiteral("Setting"));
    QCOMPARE(box->accessibleDescription(), QStringLiteral("Explanation"));
}

/**! @brief A row can carry an extra button beside its widget.
 */
void TestScrollableForm::rowButton()
{
    MScrollableForm form;
    QCheckBox *box = new QCheckBox();
    QPushButton *button = new QPushButton();
    form.addRow("Setting", box, "Help", button);

    QCOMPARE(form.rowCount(), 1);
    QVERIFY(form.isAncestorOf(box));
    QVERIFY(form.isAncestorOf(button));
}

QTEST_MAIN(TestScrollableForm)
#include "tst_scrollableform.moc"
