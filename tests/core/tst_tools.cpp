/*
** Collett - Core Tools Tests
** ==========================
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

#include "tools.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest>

using namespace Collett;

class TestTools : public QObject
{
    Q_OBJECT

private slots:
    void jsonEncodeFull();
    void jsonEncodeLimited();
    void jsonEncodeEmpty();
    void jsonEncodeStrings();
    void writeJson();
};

/**! @brief With no level limit, all containers are indented.
 */
void TestTools::jsonEncodeFull()
{
    const QJsonObject data = {
        {"a", 1},
        {"b", QJsonArray{true, QJsonValue::Null}},
        {"c", QJsonObject{{"d", 1.5}}},
    };
    const QByteArray expected = "{\n"
                                "  \"a\": 1,\n"
                                "  \"b\": [\n"
                                "    true,\n"
                                "    null\n"
                                "  ],\n"
                                "  \"c\": {\n"
                                "    \"d\": 1.5\n"
                                "  }\n"
                                "}\n";
    QCOMPARE(JsonUtils::jsonEncode(data), expected);
    QCOMPARE(JsonUtils::jsonEncode(data, -1), expected);
}

/**! @brief Containers deeper than the level limit are written on one line.
 */
void TestTools::jsonEncodeLimited()
{
    const QJsonArray content = {
        QJsonObject{{"u:fmt", "p:al"}, {"x:txt", QJsonArray{"t|A ", "t:b|B"}}},
        QJsonObject{{"u:txt", "t|C"}},
    };
    const QJsonObject data = {
        {"c:format", "CollettDocument:1.0"},
        {"c:meta", QJsonObject{{"m:created", "2026"}}},
        {"x:content", content},
    };
    QCOMPARE(
        JsonUtils::jsonEncode(data, 2),
        "{\n"
        "  \"c:format\": \"CollettDocument:1.0\",\n"
        "  \"c:meta\": {\n"
        "    \"m:created\": \"2026\"\n"
        "  },\n"
        "  \"x:content\": [\n"
        "    {\"u:fmt\": \"p:al\", \"x:txt\": [\"t|A \", \"t:b|B\"]},\n"
        "    {\"u:txt\": \"t|C\"}\n"
        "  ]\n"
        "}\n"
    );
    QCOMPARE(
        JsonUtils::jsonEncode(data, 1),
        "{\n"
        "  \"c:format\": \"CollettDocument:1.0\",\n"
        "  \"c:meta\": {\"m:created\": \"2026\"},\n"
        "  \"x:content\": [{\"u:fmt\": \"p:al\", \"x:txt\": [\"t|A \", \"t:b|B\"]}, {\"u:txt\": \"t|C\"}]\n"
        "}\n"
    );
}

/**! @brief Empty containers are kept as a pair of brackets.
 */
void TestTools::jsonEncodeEmpty()
{
    QCOMPARE(JsonUtils::jsonEncode(QJsonObject()), "{}\n");

    const QJsonObject data = {{"a", QJsonArray()}, {"b", QJsonObject()}};
    QCOMPARE(JsonUtils::jsonEncode(data), "{\n  \"a\": [],\n  \"b\": {}\n}\n");
    QCOMPARE(JsonUtils::jsonEncode(data, 1), "{\n  \"a\": [],\n  \"b\": {}\n}\n");
}

/**! @brief Structural characters and escapes inside strings are left as-is.
 */
void TestTools::jsonEncodeStrings()
{
    const QString text = QString::fromUtf8("{[a, b]: \"c\\\"}\" – æøå");
    const QJsonObject data = {{"k:{,}", text}, {"l", QJsonArray{text}}};

    const QByteArray full = JsonUtils::jsonEncode(data);
    const QByteArray limited = JsonUtils::jsonEncode(data, 1);
    QVERIFY(full.contains("\"k:{,}\": \"{[a, b]: \\\"c\\\\\\\"}\\\" – æøå\",\n"));
    QVERIFY(limited.contains("\"l\": [\"{[a, b]: \\\"c\\\\\\\"}\\\" – æøå\"]\n"));

    // The output must parse back to the same data
    QCOMPARE(QJsonDocument::fromJson(full).object(), data);
    QCOMPARE(QJsonDocument::fromJson(limited).object(), data);
}

/**! @brief Writing to file uses the encoder unless compact is set.
 */
void TestTools::writeJson()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QJsonObject data = {{"a", QJsonArray{1, 2}}};
    const QString path = tempDir.filePath("test.json");
    auto readBack = [&path]() {
        QFile file(path);
        return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
    };

    QCOMPARE(JsonUtils::writeJson(path, data, false), JsonUtilsError::NoError);
    QCOMPARE(readBack(), "{\n  \"a\": [\n    1,\n    2\n  ]\n}\n");

    QCOMPARE(JsonUtils::writeJson(path, data, false, 1), JsonUtilsError::NoError);
    QCOMPARE(readBack(), "{\n  \"a\": [1, 2]\n}\n");

    QCOMPARE(JsonUtils::writeJson(path, data, true), JsonUtilsError::NoError);
    QCOMPARE(readBack(), "{\"a\":[1,2]}");

    QCOMPARE(JsonUtils::writeJson(tempDir.filePath("none/test.json"), data, false), JsonUtilsError::FileError);
}

QTEST_GUILESS_MAIN(TestTools)
#include "tst_tools.moc"
