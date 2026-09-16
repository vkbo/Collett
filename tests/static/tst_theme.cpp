/*
** Collett - Theme Class Tests
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

#include "collett.h"
#include "settings.h"
#include "theme.h"

#include <QDir>
#include <QtTest>

using namespace Collett;

class TestTheme : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void scanThemes();
    void loadByKey();
    void loadFromSettings();
};

/**! @brief Isolate QSettings and point the theme at the source assets.
 *
 * The test binary does not sit next to an assets folder, so the themes are
 * scanned from the repository instead.
 */
void TestTheme::initTestCase()
{
    QCoreApplication::setOrganizationName("CollettTest");
    QCoreApplication::setApplicationName("tst_theme");
    Theme::instance()->scanThemes(QDir(QStringLiteral(ASSETS_DIR "/themes")));
}

void TestTheme::cleanupTestCase()
{
    Theme::destroy();
    Settings::destroy();
}

/**! @brief The scan finds the bundled themes, sorted with defaults first, and splits them by mode.
 */
void TestTheme::scanThemes()
{
    Theme *theme = Theme::instance();
    const QList<ThemeEntry> themes = theme->themes();
    QCOMPARE(themes.count(), 3);
    QCOMPARE(themes.at(0).key, QStringLiteral("default_dark"));
    QCOMPARE(themes.at(1).key, QStringLiteral("default_light"));
    QCOMPARE(themes.at(2).key, QStringLiteral("dracula"));
    QCOMPARE(themes.at(2).name, QStringLiteral("Dracula"));

    const QList<ThemeEntry> light = theme->lightThemes();
    QCOMPARE(light.count(), 1);
    QCOMPARE(light.at(0).key, QStringLiteral("default_light"));
    QVERIFY(!light.at(0).dark);

    const QList<ThemeEntry> dark = theme->darkThemes();
    QCOMPARE(dark.count(), 2);
    QCOMPARE(dark.at(0).key, QStringLiteral("default_dark"));
    QCOMPARE(dark.at(1).key, QStringLiteral("dracula"));
    QVERIFY(dark.at(1).dark);

    QVERIFY(theme->hasTheme("dracula"));
    QVERIFY(!theme->hasTheme("missing"));
}

/**! @brief Loading by key applies the theme, and unknown keys are rejected.
 */
void TestTheme::loadByKey()
{
    Theme *theme = Theme::instance();

    QVERIFY(theme->loadTheme("dracula"));
    QCOMPARE(theme->currentTheme(), QStringLiteral("dracula"));
    QVERIFY(theme->isDark());

    QVERIFY(theme->loadTheme("default_light"));
    QCOMPARE(theme->currentTheme(), QStringLiteral("default_light"));
    QVERIFY(!theme->isDark());

    QVERIFY(!theme->loadTheme("missing"));
    QCOMPARE(theme->currentTheme(), QStringLiteral("default_light"));
}

/**! @brief The theme mode picks the light or dark setting, with fallback to the defaults.
 */
void TestTheme::loadFromSettings()
{
    Theme *theme = Theme::instance();
    Settings *settings = Settings::instance();

    settings->setThemeMode(ThemeMode::DarkTheme);
    settings->setDarkTheme("dracula");
    QVERIFY(theme->loadTheme());
    QCOMPARE(theme->currentTheme(), QStringLiteral("dracula"));

    settings->setThemeMode(ThemeMode::LightTheme);
    settings->setLightTheme("default_light");
    QVERIFY(theme->loadTheme());
    QCOMPARE(theme->currentTheme(), QStringLiteral("default_light"));

    // Loading the current theme again is a no-op
    QVERIFY(!theme->loadTheme());
    QCOMPARE(theme->currentTheme(), QStringLiteral("default_light"));

    // An unknown theme falls back to the default and corrects the setting
    settings->setThemeMode(ThemeMode::DarkTheme);
    settings->setDarkTheme("missing");
    QVERIFY(theme->loadTheme());
    QCOMPARE(theme->currentTheme(), QStringLiteral("default_dark"));
    QCOMPARE(settings->darkTheme(), QStringLiteral("default_dark"));
}

QTEST_MAIN(TestTheme)
#include "tst_theme.moc"
