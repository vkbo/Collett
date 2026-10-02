/*
** Collett - ProjectData Class Tests
** =================================
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

#include "projectdata.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QtTest>

using namespace Collett;

class TestProjectData : public QObject
{
    Q_OBJECT

private slots:
    void spellLanguageDefault();
    void spellLanguagePack();
    void spellLanguageUnpack();
};

/**! @brief A new project has no spell language override.
 */
void TestProjectData::spellLanguageDefault()
{
    ProjectData data;
    QVERIFY(!data.hasSpellLanguage());
    QVERIFY(data.spellLanguage().isEmpty());

    data.setSpellLanguage("  en_GB  ");
    QVERIFY(data.hasSpellLanguage());
    QCOMPARE(data.spellLanguage(), QStringLiteral("en_GB"));

    data.setSpellLanguage("");
    QVERIFY(!data.hasSpellLanguage());
}

/**! @brief No override is written as null, an override as a string.
 */
void TestProjectData::spellLanguagePack()
{
    ProjectData data;
    QJsonObject json;

    data.pack(json);
    QJsonObject settings = json.value("c:settings").toObject();
    QVERIFY(settings.contains("u:spellLanguage"));
    QVERIFY(settings.value("u:spellLanguage").isNull());

    data.setSpellLanguage("nb_NO");
    data.pack(json);
    settings = json.value("c:settings").toObject();
    QCOMPARE(settings.value("u:spellLanguage").toString(), QStringLiteral("nb_NO"));
}

/**! @brief Null, missing, and string values all unpack as expected.
 */
void TestProjectData::spellLanguageUnpack()
{
    ProjectData data;
    QJsonObject json, settings;

    // String value
    settings["u:spellLanguage"] = "en_US";
    json["c:settings"] = settings;
    data.unpack(json);
    QCOMPARE(data.spellLanguage(), QStringLiteral("en_US"));

    // Null value clears a previous override
    settings["u:spellLanguage"] = QJsonValue();
    json["c:settings"] = settings;
    data.unpack(json);
    QVERIFY(!data.hasSpellLanguage());

    // Missing key also means no override
    data.setSpellLanguage("en_US");
    data.unpack(QJsonObject());
    QVERIFY(!data.hasSpellLanguage());
}

QTEST_MAIN(TestProjectData)
#include "tst_projectdata.moc"
