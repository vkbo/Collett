/*
** Collett - Settings Class Tests
** ==============================
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

#include "settings.h"

#include <QLocale>
#include <QtTest>

using namespace Collett;

class TestSettings : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void spellLanguage();
};

/**! @brief Isolate QSettings from a real Collett install.
 *
 * The org and app names decide where QSettings reads from. Using test names
 * means no stored values exist, so the defaults are what get tested.
 */
void TestSettings::initTestCase()
{
    QCoreApplication::setOrganizationName("CollettTest");
    QCoreApplication::setApplicationName("tst_settings");
}

void TestSettings::cleanupTestCase()
{
    Settings::destroy();
}

/**! @brief The spell language defaults to the system locale and can be changed.
 */
void TestSettings::spellLanguage()
{
    Settings *settings = Settings::instance();
    QCOMPARE(settings->spellLanguage(), QLocale::system().name());

    settings->setSpellLanguage("nb_NO");
    QCOMPARE(settings->spellLanguage(), QStringLiteral("nb_NO"));
}

QTEST_MAIN(TestSettings)
#include "tst_settings.moc"
