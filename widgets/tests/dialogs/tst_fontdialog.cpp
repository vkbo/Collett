/*
** Collett - Font Dialog Tests
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

#include "fontdialog.h"
#include "mpushbutton.h"
#include "settings.h"
#include "theme.h"

#include <QDialogButtonBox>
#include <QFont>
#include <QtTest>

using namespace Collett;

class TestFontDialog : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void buttons();
    void windowSize();
};

void TestFontDialog::initTestCase()
{
    QCoreApplication::setOrganizationName("CollettTest");
    QCoreApplication::setApplicationName("tst_fontdialog");
}

void TestFontDialog::cleanupTestCase()
{
    Theme::destroy();
    Settings::destroy();
}

/**! @brief The Qt buttons are replaced by the standard OK and Cancel buttons.
 */
void TestFontDialog::buttons()
{
    FontDialog dialog{QFont()};
    QDialogButtonBox *box = dialog.findChild<QDialogButtonBox *>();
    QVERIFY(box != nullptr);
    QCOMPARE(box->buttons().count(), 2);

    const QList<MPushButton *> buttons = dialog.findChildren<MPushButton *>();
    QCOMPARE(buttons.count(), 2);
    QCOMPARE(box->buttonRole(buttons.at(0)), QDialogButtonBox::AcceptRole);
    QCOMPARE(box->buttonRole(buttons.at(1)), QDialogButtonBox::RejectRole);
}

/**! @brief The dialog opens at the stored size and stores it again on close.
 */
void TestFontDialog::windowSize()
{
    Settings *settings = Settings::instance();
    settings->setFontWindowSize(QSize(640, 480));

    FontDialog dialog{QFont()};
    QCOMPARE(dialog.size(), QSize(640, 480));

    dialog.resize(720, 500);
    dialog.reject();
    QCOMPARE(settings->fontWindowSize(), QSize(720, 500));
    QCOMPARE(dialog.result(), int(QDialog::Rejected));
}

QTEST_MAIN(TestFontDialog)
#include "tst_fontdialog.moc"
