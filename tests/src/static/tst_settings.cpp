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
#include <QSignalSpy>
#include <QtTest>

using namespace Collett;

class TestSettings : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void editorAutoSave();
    void spellLanguage();
    void tabWidth();
    void autoIndent();
    void fonts();
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

/**! @brief The auto save interval has a default and can be changed.
 */
void TestSettings::editorAutoSave()
{
    Settings *settings = Settings::instance();
    QCOMPARE(settings->editorAutoSave(), 30);

    settings->setEditorAutoSave(60);
    QCOMPARE(settings->editorAutoSave(), 60);
}

/**! @brief Automatic indent is on by default, and can be turned off.
 */
void TestSettings::autoIndent()
{
    Settings *settings = Settings::instance();
    QCOMPARE(settings->textAutoIndent(), true);

    QSignalSpy spy(settings, &Settings::textAutoIndentChanged);
    settings->setTextAutoIndent(false);
    QCOMPARE(settings->textAutoIndent(), false);
    settings->setTextAutoIndent(false);
    QCOMPARE(spy.count(), 1);
}

/**! @brief The spell language defaults to the system locale and can be changed.
 */
void TestSettings::spellLanguage()
{
    Settings *settings = Settings::instance();
    QCOMPARE(settings->spellLanguage(), QLocale::system().name());

    settings->setSpellLanguage(" nb_NO ");
    QCOMPARE(settings->spellLanguage(), QStringLiteral("nb_NO"));
}

/**! @brief The tab width has a default and drives the text format.
 */
void TestSettings::tabWidth()
{
    Settings *settings = Settings::instance();
    QCOMPARE(settings->textFormat().tabWidth, 40.0);

    settings->setTextTabWidth(60.0);
    QCOMPARE(settings->textFormat().tabWidth, 60.0);
}

/**! @brief The text font drives the text format size, and the fonts have defaults.
 */
void TestSettings::fonts()
{
    Settings *settings = Settings::instance();
    QCOMPARE(settings->textFont().pointSizeF(), 13.0);
    QCOMPARE(settings->textFormat().fontSize, 13.0);

    QFont font = settings->textFont();
    font.setPointSizeF(16.0);
    settings->setTextFont(font);
    QCOMPARE(settings->textFont().pointSizeF(), 16.0);
    QCOMPARE(settings->textFormat().fontSize, 16.0);
    QCOMPARE(settings->textFormat().charDefault.fontPointSize(), 16.0);
    QCOMPARE(settings->textFormat().charDefault.fontFamilies().toStringList().first(), font.family());
}

QTEST_MAIN(TestSettings)
#include "tst_settings.moc"
