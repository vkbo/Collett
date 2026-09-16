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
#include "settings.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QFont>
#include <QtTest>

using namespace Collett;

class TestDocument : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void packSimpleFormatting();
    void roundTripIsStable();
    void commentBlock();
    void updatedTimestamp();
    void refreshTextFormat();
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

/**! @brief A comment block is stored with type "c" and restored with its property.
 */
void TestDocument::commentBlock()
{
    Document original;
    QTextCursor cursor(&original);

    cursor.insertText("Text");
    QTextBlockFormat comment;
    comment.setProperty(BlockTypeProperty, CommentBlock);
    cursor.insertBlock(comment);
    cursor.insertText("A comment");

    QJsonObject data;
    original.pack(data);
    QJsonArray content = data.value("x:content").toArray();
    QCOMPARE(content.size(), 2);
    QCOMPARE(content.at(0).toObject().value("u:fmt").toString(), QStringLiteral("p:al"));
    QCOMPARE(content.at(1).toObject().value("u:fmt").toString(), QStringLiteral("c:al"));

    Document restored;
    restored.unpack(data);
    QCOMPARE(restored.blockCount(), 2);
    QTextBlock second = restored.firstBlock().next();
    QCOMPARE(second.text(), QStringLiteral("A comment"));
    QCOMPARE(second.blockFormat().intProperty(BlockTypeProperty), int(CommentBlock));
    QCOMPARE(restored.firstBlock().blockFormat().intProperty(BlockTypeProperty), int(TextBlock));
}

/**! @brief The updated timestamp only changes when the content has changed.
 */
void TestDocument::updatedTimestamp()
{
    const QString oldTime = QStringLiteral("2020-01-01T00:00:00");

    // A new document gets a timestamp on first pack
    Document fresh;
    QTextCursor cursor(&fresh);
    cursor.insertText("Text");
    QJsonObject data;
    fresh.pack(data);
    QVERIFY(!data.value("c:meta").toObject().value("m:updated").toString().isEmpty());

    // Give the stored document an old timestamp
    QJsonObject meta = data.value("c:meta").toObject();
    meta["m:updated"] = oldTime;
    data["c:meta"] = meta;

    // Loading and saving without edits keeps it
    Document loaded;
    loaded.unpack(data);
    QVERIFY(!loaded.isModified());
    QCOMPARE(loaded.updatedTime(), oldTime);
    QJsonObject saved;
    loaded.pack(saved);
    QCOMPARE(saved.value("c:meta").toObject().value("m:updated").toString(), oldTime);

    // An edit moves it, and it stays put after the modified flag is cleared
    // the way Project does after writing to disk
    QTextCursor edit(&loaded);
    edit.movePosition(QTextCursor::End);
    edit.insertText("!");
    QVERIFY(loaded.isModified());
    loaded.pack(saved);
    QString newTime = saved.value("c:meta").toObject().value("m:updated").toString();
    QVERIFY(newTime != oldTime);
    loaded.setModified(false);
    loaded.pack(saved);
    QCOMPARE(saved.value("c:meta").toObject().value("m:updated").toString(), newTime);
}

/**! @brief Changing the text font restyles a loaded document in place.
 *
 * Headers scale with the base size, fragment flags like bold survive, the
 * document takes the new family, and the modified state is untouched.
 */
void TestDocument::refreshTextFormat()
{
    Settings *settings = Settings::instance();
    QFont font = settings->textFont();
    font.setPointSizeF(13.0);
    settings->setTextFont(font);

    QJsonObject header;
    header["u:fmt"] = "h1";
    header["u:txt"] = "t|Title";
    QJsonObject paragraph;
    paragraph["u:fmt"] = "p";
    paragraph["x:txt"] = QJsonArray({"t|Plain ", "t:b|bold"});
    QJsonObject data;
    data["c:format"] = "CollettDocument";
    data["x:content"] = QJsonArray({header, paragraph});

    Document doc;
    doc.unpack(data);
    QCOMPARE(doc.blockCount(), 2);
    QCOMPARE(doc.firstBlock().begin().fragment().charFormat().fontPointSize(), 26.0);
    QVERIFY(!doc.isModified());

    font.setPointSizeF(20.0);
    settings->setTextFont(font);

    const QTextBlock first = doc.firstBlock();
    const QTextBlock second = first.next();
    QCOMPARE(first.begin().fragment().charFormat().fontPointSize(), 40.0);
    QCOMPARE(first.begin().fragment().charFormat().fontWeight(), int(QFont::Bold));

    QTextBlock::iterator it = second.begin();
    QCOMPARE(it.fragment().text(), QStringLiteral("Plain "));
    QCOMPARE(it.fragment().charFormat().fontPointSize(), 20.0);
    QCOMPARE(it.fragment().charFormat().fontWeight(), int(QFont::Normal));
    QCOMPARE(it.fragment().charFormat().fontFamilies().toStringList().first(), font.family());
    ++it;
    QCOMPARE(it.fragment().text(), QStringLiteral("bold"));
    QCOMPARE(it.fragment().charFormat().fontPointSize(), 20.0);
    QCOMPARE(it.fragment().charFormat().fontWeight(), int(QFont::Bold));

    QCOMPARE(doc.defaultFont().pointSizeF(), 20.0);
    QVERIFY(!doc.isModified());

    // Restore the default for any test that follows
    font.setPointSizeF(13.0);
    settings->setTextFont(font);
}

QTEST_MAIN(TestDocument)
#include "tst_document.moc"
