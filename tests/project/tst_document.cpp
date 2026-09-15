/*
** Collett - Document Class Tests
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

#include "document.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QtTest>

using namespace Collett;

class TestDocument : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void packSimpleFormatting();
    void roundTripIsStable();
};

/**! @brief Set an org/app name so the Settings singleton's QSettings works.
 *
 * Document::unpack() reads its formats from Settings::instance(), which
 * constructs a default QSettings. Naming the application keeps QSettings happy
 * and isolates any stored values from a real Collett install.
 */
void TestDocument::initTestCase()
{
    QCoreApplication::setOrganizationName("CollettTest");
    QCoreApplication::setApplicationName("tst_document");
}

/**! @brief Verify the serialization contract for a mixed-format paragraph.
 *
 * A single paragraph holding a plain fragment followed by a bold fragment must
 * pack into one content block with the expected format markers and text.
 */
void TestDocument::packSimpleFormatting()
{
    Document doc;
    QTextCursor cursor(&doc);
    cursor.insertText("Hello ");
    QTextCharFormat bold;
    bold.setFontWeight(QFont::Bold);
    cursor.insertText("world", bold);

    QJsonObject data;
    doc.pack(data);

    QCOMPARE(data.value("c:format").toString(), QStringLiteral("CollettDocument"));

    QJsonArray content = data.value("x:content").toArray();
    QCOMPARE(content.size(), 1);

    QJsonObject block = content.at(0).toObject();
    QCOMPARE(block.value("u:fmt").toString(), QStringLiteral("p:al"));

    // Two differently formatted fragments are stored as an array under x:txt.
    QJsonArray frags = block.value("x:txt").toArray();
    QCOMPARE(frags.size(), 2);
    QCOMPARE(frags.at(0).toString(), QStringLiteral("t|Hello "));
    QCOMPARE(frags.at(1).toString(), QStringLiteral("t:b|world"));
}

/**! @brief pack()/unpack() must reach a stable fixed point.
 *
 * The first unpack normalizes a document against the Settings defaults (e.g.
 * heading and paragraph styles), so pack of the original and pack of the
 * round-tripped document may legitimately differ. From the second pass onward
 * the content must be identical, which exercises both directions together.
 */
void TestDocument::roundTripIsStable()
{
    Document original;
    QTextCursor cursor(&original);

    QTextBlockFormat heading;
    heading.setHeadingLevel(1);
    cursor.setBlockFormat(heading);
    cursor.insertText("Title");

    cursor.insertBlock(QTextBlockFormat());
    cursor.insertText("Body ");
    QTextCharFormat italic;
    italic.setFontItalic(true);
    cursor.insertText("italic", italic);

    QJsonObject data1;
    original.pack(data1);

    Document second;
    second.unpack(data1);
    QJsonObject data2;
    second.pack(data2);

    Document third;
    third.unpack(data2);
    QJsonObject data3;
    third.pack(data3);

    QCOMPARE(data3.value("x:content"), data2.value("x:content"));
}

QTEST_MAIN(TestDocument)
#include "tst_document.moc"
