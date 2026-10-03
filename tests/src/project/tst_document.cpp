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

#include <QFont>
#include <QJsonArray>
#include <QJsonObject>
#include <QTextBlock>
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
    void emptyContent();
    void structure();
    void insertItem();
    void removeItem();
    void mergeItem();
    void moveItems();
    void moveItemsKeepsFormat();
    void setItemValues();
    void normalize();
    void refreshTextFormat();

private:
    static Document::Item item(const QString &handle, const QString &title = QString(), ItemLevel level = ItemLevel::SceneLevel);
    static QJsonArray paragraphs(const QString &text);
    static void build(Document &doc, const QString &layout);
    static QString layout(const Document &doc);
};

/**! @brief Set an org/app name so the Settings singleton's QSettings works.
 *
 * The document reads its formats from Settings::instance(), which constructs
 * a default QSettings. Naming the application keeps QSettings happy and
 * isolates any stored values from a real Collett install.
 */
void TestDocument::initTestCase()
{
    QCoreApplication::setOrganizationName("CollettTest");
    QCoreApplication::setApplicationName("tst_document");
}

Document::Item TestDocument::item(const QString &handle, const QString &title, ItemLevel level)
{
    Document::Item item;
    item.handle = handle;
    item.title = title;
    item.level = level;
    return item;
}

/**! @brief Plain paragraphs in the format of the files, one per line.
 */
QJsonArray TestDocument::paragraphs(const QString &text)
{
    QJsonArray content;
    for (const QString &line : text.split('\n'))
        content.append(QJsonObject({{"u:fmt", "p:al"}, {"u:txt", "t|" + line}}));
    return content;
}

/**! @brief Build a document from a layout like "a:1,2 b: c:3", where each
 * word is a document with its handle and the lines of its text. The title of
 * each document is its handle in upper case.
 */
void TestDocument::build(Document &doc, const QString &layout)
{
    for (const QString &word : layout.split(' ')) {
        const QString handle = word.section(':', 0, 0);
        const QString text = word.section(':', 1).replace(',', '\n');
        doc.appendItem(item(handle, handle.toUpper()), text.isEmpty() ? QJsonArray() : paragraphs(text));
    }
}

/**! @brief The layout of a document, as given to build. A block outside a
 * document is written first, with no handle.
 */
QString TestDocument::layout(const Document &doc)
{
    QStringList words;
    QStringList lines;
    QString handle;
    bool first = true;
    auto flush = [&]() {
        if (!first) words.append(handle + ":" + lines.join(','));
        lines.clear();
    };
    for (QTextBlock block = doc.begin(); block.isValid(); block = block.next()) {
        if (Document::isTitle(block)) {
            flush();
            first = false;
            handle = Document::itemOf(block).handle;
            if (block.text() != handle.toUpper()) handle += "=" + block.text();
        } else {
            if (first) {
                first = false;
                handle = QString();
            }
            lines.append(block.text());
        }
    }
    flush();
    return words.join(' ');
}

/**! @brief Verify the serialization contract for a mixed-format paragraph.
 *
 * A single paragraph holding a plain fragment followed by a bold fragment must
 * pack into one content block with the expected format markers and text. The
 * title is not part of the content.
 */
void TestDocument::packSimpleFormatting()
{
    Document doc;
    doc.appendItem(item("a", "Title"), QJsonArray());
    QTextCursor cursor(&doc);
    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());
    cursor.insertText("Hello ");
    QTextCharFormat bold;
    bold.setFontWeight(QFont::Bold);
    cursor.insertText("world", bold);

    QJsonArray content = doc.packContent("a");
    QCOMPARE(content.size(), 1);

    QJsonObject block = content.at(0).toObject();
    QCOMPARE(block.value("u:fmt").toString(), QStringLiteral("p:al"));

    // Two differently formatted fragments are stored as an array under x:txt.
    QJsonArray frags = block.value("x:txt").toArray();
    QCOMPARE(frags.size(), 2);
    QCOMPARE(frags.at(0).toString(), QStringLiteral("t|Hello "));
    QCOMPARE(frags.at(1).toString(), QStringLiteral("t:b|world"));
}

/**! @brief Packing and unpacking must reach a stable fixed point.
 *
 * The first unpack normalizes the content against the Settings defaults
 * (e.g. heading and paragraph styles), so the content given and the content
 * packed may legitimately differ. From the second pass onward the content
 * must be identical, which exercises both directions together.
 */
void TestDocument::roundTripIsStable()
{
    QJsonObject heading;
    heading["u:fmt"] = "h1";
    heading["u:txt"] = "t|Title";
    QJsonObject body;
    body["u:fmt"] = "p:ac:ti";
    body["x:txt"] = QJsonArray({"t|Body ", "t:i|italic"});

    Document first;
    first.appendItem(item("a"), QJsonArray({heading, body}));
    first.appendItem(item("b"), paragraphs("Other"));
    const QJsonArray data1 = first.packContent("a");

    Document second;
    second.appendItem(item("a"), data1);
    const QJsonArray data2 = second.packContent("a");

    Document third;
    third.appendItem(item("a"), data2);
    QCOMPARE(third.packContent("a"), data2);
    QCOMPARE(first.packContent("b"), paragraphs("Other"));
}

/**! @brief A document without text, or with one empty paragraph, has no
 * content.
 */
void TestDocument::emptyContent()
{
    Document doc;
    build(doc, "a: b:x c");
    QTextCursor cursor(&doc);
    cursor.setPosition(doc.itemEnd("a"));
    cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());
    QCOMPARE(layout(doc), QStringLiteral("a: b:x c:"));
    QCOMPARE(doc.packContent("a"), QJsonArray());
    QCOMPARE(doc.packContent("b"), paragraphs("x"));
    QCOMPARE(doc.packContent("c"), QJsonArray());
    QCOMPARE(doc.packContent("nope"), QJsonArray());
}

/**! @brief The documents are found from the title blocks.
 */
void TestDocument::structure()
{
    Document doc;
    doc.appendItem(item("a", "One", ItemLevel::ChapterLevel), QJsonArray());
    Document::Item scene = item("b", "", ItemLevel::SceneLevel);
    scene.hardBreak = true;
    doc.appendItem(scene, paragraphs("Alpha\nBeta"));
    Document::Item chapter = item("c", "Two", ItemLevel::ChapterLevel);
    chapter.numbered = false;
    doc.appendItem(chapter, paragraphs("Gamma"));
    QVERIFY(!doc.isModified());
    QVERIFY(!doc.isUndoAvailable());

    const QList<Document::Item> items = doc.items();
    QCOMPARE(items.size(), 3);
    QCOMPARE(items.at(0).handle, QStringLiteral("a"));
    QCOMPARE(items.at(0).title, QStringLiteral("One"));
    QCOMPARE(items.at(0).level, ItemLevel::ChapterLevel);
    QCOMPARE(items.at(1).title, QString());
    QVERIFY(items.at(1).hardBreak);
    QVERIFY(items.at(1).numbered);
    QVERIFY(!items.at(2).numbered);

    QCOMPARE(doc.itemText("a"), QString());
    QCOMPARE(doc.itemText("b"), QStringLiteral("Alpha\nBeta"));
    QCOMPARE(doc.handleAt(0), QStringLiteral("a"));
    QCOMPARE(doc.handleAt(doc.titleBlock("b").position()), QStringLiteral("b"));
    QCOMPARE(doc.handleAt(doc.itemEnd("b")), QStringLiteral("b"));
    QCOMPARE(doc.handleAt(doc.characterCount() - 1), QStringLiteral("c"));
    QCOMPARE(doc.itemEnd("a"), 3);
    QVERIFY(!doc.titleBlock("nope").isValid());
    QCOMPARE(doc.snapshotItem("b").size(), 2);

    // Titles are not headings, and get the title font for their level
    const QTextBlock title = doc.titleBlock("a");
    QCOMPARE(title.blockFormat().headingLevel(), 0);
    QCOMPARE(title.charFormat().fontPointSize(), Document::titleCharFormat(ItemLevel::ChapterLevel).fontPointSize());
}

/**! @brief A new document splits a paragraph, or goes before or after it,
 * or takes over an empty one. A split in a title is refused. Each split is
 * one undo step.
 */
void TestDocument::insertItem()
{
    Document doc;
    build(doc, "a:Alpha,,Beta");
    const int alpha = doc.titleBlock("a").next().position();

    QCOMPARE(doc.insertItem(alpha + 2, item("b")), alpha + 3);
    QCOMPARE(layout(doc), QStringLiteral("a:Al b=:pha,,Beta"));
    doc.undo();
    QCOMPARE(layout(doc), QStringLiteral("a:Alpha,,Beta"));

    QVERIFY(doc.insertItem(alpha, item("b")) > 0);
    QCOMPARE(layout(doc), QStringLiteral("a: b=:Alpha,,Beta"));
    doc.undo();

    QVERIFY(doc.insertItem(alpha + 5, item("b")) > 0);
    QCOMPARE(layout(doc), QStringLiteral("a:Alpha b=:,Beta"));
    doc.undo();

    QVERIFY(doc.insertItem(alpha + 6, item("b")) > 0);
    QCOMPARE(layout(doc), QStringLiteral("a:Alpha b=:Beta"));
    doc.undo();

    QVERIFY(doc.insertItem(doc.characterCount() - 1, item("b", "Title")) > 0);
    QCOMPARE(layout(doc), QStringLiteral("a:Alpha,,Beta b=Title:"));
    QCOMPARE(doc.items().last().title, QStringLiteral("Title"));
    doc.undo();

    QCOMPARE(doc.insertItem(1, item("b")), -1);
    QCOMPARE(layout(doc), QStringLiteral("a:Alpha,,Beta"));
}

/**! @brief A document is removed with its title and text, which is one undo
 * step. The last document cannot be removed.
 */
void TestDocument::removeItem()
{
    Document doc;
    build(doc, "a:1 b:2,3 c: d:4");

    QVERIFY(doc.removeItem("b"));
    QCOMPARE(layout(doc), QStringLiteral("a:1 c: d:4"));
    doc.undo();
    QCOMPARE(layout(doc), QStringLiteral("a:1 b:2,3 c: d:4"));
    QCOMPARE(doc.items().at(1).title, QStringLiteral("B"));

    QVERIFY(doc.removeItem("a"));
    QCOMPARE(layout(doc), QStringLiteral("b:2,3 c: d:4"));
    QVERIFY(doc.removeItem("d"));
    QCOMPARE(layout(doc), QStringLiteral("b:2,3 c:"));
    QVERIFY(doc.removeItem("c"));
    QCOMPARE(layout(doc), QStringLiteral("b:2,3"));
    QVERIFY(!doc.removeItem("b"));
    QVERIFY(!doc.removeItem("nope"));
}

/**! @brief Merging removes the title, so the text joins the document before,
 * and returns where the merged text starts.
 */
void TestDocument::mergeItem()
{
    Document doc;
    build(doc, "a:1 b:2,3 c: d:4");

    const int start = doc.titleBlock("b").position();
    QCOMPARE(doc.mergeItem("b"), start);
    QCOMPARE(layout(doc), QStringLiteral("a:1,2,3 c: d:4"));
    QCOMPARE(doc.findBlock(start).text(), QStringLiteral("2"));
    doc.undo();
    QCOMPARE(layout(doc), QStringLiteral("a:1 b:2,3 c: d:4"));

    // Without text, the title is removed, and the cursor goes to the end of
    // the text before
    QCOMPARE(doc.mergeItem("c"), doc.itemEnd("b"));
    QCOMPARE(layout(doc), QStringLiteral("a:1 b:2,3 d:4"));

    QCOMPARE(doc.mergeItem("a"), -1);
    QCOMPARE(doc.mergeItem("nope"), -1);
}

/**! @brief Documents move as blocks of whole documents, in one undo step.
 */
void TestDocument::moveItems()
{
    Document doc;
    build(doc, "a:1 b:2,3 c: d:4");
    const QString start = layout(doc);

    QVERIFY(doc.moveItems(1, 1, 3));
    QCOMPARE(layout(doc), QStringLiteral("a:1 c: b:2,3 d:4"));
    doc.undo();
    QCOMPARE(layout(doc), start);

    QVERIFY(doc.moveItems(1, 2, 4));
    QCOMPARE(layout(doc), QStringLiteral("a:1 d:4 b:2,3 c:"));
    doc.undo();

    QVERIFY(doc.moveItems(2, 2, 0));
    QCOMPARE(layout(doc), QStringLiteral("c: d:4 a:1 b:2,3"));
    doc.undo();

    QVERIFY(doc.moveItems(0, 1, 4));
    QCOMPARE(layout(doc), QStringLiteral("b:2,3 c: d:4 a:1"));
    doc.undo();

    QVERIFY(doc.moveItems(3, 1, 0));
    QCOMPARE(layout(doc), QStringLiteral("d:4 a:1 b:2,3 c:"));
    doc.undo();
    QCOMPARE(layout(doc), start);

    QVERIFY(!doc.moveItems(1, 1, 1));
    QVERIFY(!doc.moveItems(1, 1, 2));
    QVERIFY(!doc.moveItems(1, 2, 2));
    QVERIFY(!doc.moveItems(1, 1, 9));
    QVERIFY(!doc.moveItems(3, 2, 0));
    QCOMPARE(layout(doc), start);
}

/**! @brief Moved documents keep their values and the format of their text.
 */
void TestDocument::moveItemsKeepsFormat()
{
    Document doc;
    Document::Item chapter = item("a", "One", ItemLevel::ChapterLevel);
    chapter.numbered = false;
    doc.appendItem(chapter, paragraphs("x"));
    QJsonObject centred;
    centred["u:fmt"] = "h2:ac";
    centred["x:txt"] = QJsonArray({"t|Plain ", "t:b|bold"});
    doc.appendItem(item("b", "Two"), QJsonArray({centred}));
    const QJsonArray content = doc.packContent("b");

    QVERIFY(doc.moveItems(1, 1, 0));
    QCOMPARE(doc.items().at(0).handle, QStringLiteral("b"));
    QCOMPARE(doc.packContent("b"), content);
    QVERIFY(doc.moveItems(1, 1, 0));
    QCOMPARE(doc.items().at(0).handle, QStringLiteral("a"));
    QCOMPARE(doc.items().at(0).level, ItemLevel::ChapterLevel);
    QVERIFY(!doc.items().at(0).numbered);
    QCOMPARE(doc.titleBlock("a").charFormat().fontPointSize(), Document::titleCharFormat(ItemLevel::ChapterLevel).fontPointSize());
    QCOMPARE(doc.packContent("b"), content);
    QCOMPARE(doc.packContent("a"), paragraphs("x"));
}

/**! @brief The level and break settings, and the title text, are edits that
 * can be undone.
 */
void TestDocument::setItemValues()
{
    Document doc;
    build(doc, "a:1 b:2");
    const qreal before = doc.titleBlock("b").blockFormat().topMargin();

    Document::Item values = item("b", "ignored", ItemLevel::ChapterLevel);
    QVERIFY(doc.setItemValues(values));
    QCOMPARE(doc.items().at(1).level, ItemLevel::ChapterLevel);
    QCOMPARE(doc.items().at(1).title, QStringLiteral("B"));
    QVERIFY(doc.titleBlock("b").blockFormat().topMargin() > before);
    QCOMPARE(doc.titleBlock("b").charFormat().fontPointSize(), Document::titleCharFormat(ItemLevel::ChapterLevel).fontPointSize());
    QVERIFY(!doc.setItemValues(values));
    doc.undo();
    QCOMPARE(doc.items().at(1).level, ItemLevel::SceneLevel);
    QCOMPARE(doc.titleBlock("b").blockFormat().topMargin(), before);

    QVERIFY(doc.setItemTitle("b", "New"));
    QCOMPARE(doc.items().at(1).title, QStringLiteral("New"));
    QVERIFY(!doc.setItemTitle("b", "New"));
    doc.undo();
    QCOMPARE(doc.items().at(1).title, QStringLiteral("B"));
}

/**! @brief A line break in a title makes a second title with the same
 * handle, which becomes a paragraph. Text left before the first title gets
 * a new title. Each fix joins the edit that caused it in the undo history.
 */
void TestDocument::normalize()
{
    Document doc;
    build(doc, "a:1 b:2");
    QVERIFY(!doc.normalize("n"));

    QTextCursor cursor(&doc);
    cursor.setPosition(doc.titleBlock("b").position() + 1);
    cursor.insertText("x\ny");
    QCOMPARE(doc.items().size(), 2);
    QVERIFY(doc.normalize("n"));
    QCOMPARE(layout(doc), QStringLiteral("a:1 b=Bx:y,2"));
    QVERIFY(!Document::isTitle(doc.titleBlock("b").next()));
    QCOMPARE(doc.titleBlock("b").next().charFormat().fontPointSize(), Settings::instance()->textFormat().charParagraph.fontPointSize());
    doc.undo();
    QCOMPARE(layout(doc), QStringLiteral("a:1 b:2"));

    cursor.setPosition(0);
    cursor.setPosition(doc.titleBlock("b").next().position() + 1, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    QCOMPARE(doc.items().size(), 0);
    QVERIFY(doc.normalize("n"));
    QCOMPARE(layout(doc), QStringLiteral("n=:"));
    QCOMPARE(doc.items().size(), 1);
    QCOMPARE(doc.items().at(0).level, ItemLevel::SceneLevel);
    doc.undo();
    QCOMPARE(layout(doc), QStringLiteral("a:1 b:2"));
}

/**! @brief Changing the text font restyles a loaded document in place.
 *
 * Titles and headers scale with the heading font, fragment flags like bold
 * survive, the document takes the new family, and the modified state is
 * untouched.
 */
void TestDocument::refreshTextFormat()
{
    Settings *settings = Settings::instance();
    QFont font = settings->textFont();
    font.setPointSizeF(13.0);
    settings->setTextFont(font);
    QFont heading = settings->headingFont();
    heading.setPointSizeF(13.0);
    settings->setHeadingFont(heading);

    QJsonObject header;
    header["u:fmt"] = "h1";
    header["u:txt"] = "t|Title";
    QJsonObject paragraph;
    paragraph["u:fmt"] = "p";
    paragraph["x:txt"] = QJsonArray({"t|Plain ", "t:b|bold"});
    Document doc;
    doc.appendItem(item("a", "Name", ItemLevel::ChapterLevel), QJsonArray({header, paragraph}));
    QCOMPARE(doc.blockCount(), 3);
    QCOMPARE(doc.firstBlock().begin().fragment().charFormat().fontPointSize(), 13.0 * 1.7);
    QCOMPARE(doc.firstBlock().next().begin().fragment().charFormat().fontPointSize(), 26.0);
    QVERIFY(!doc.isModified());

    font.setPointSizeF(20.0);
    settings->setTextFont(font);
    heading.setPointSizeF(20.0);
    settings->setHeadingFont(heading);

    const QTextBlock title = doc.firstBlock();
    const QTextBlock first = title.next();
    const QTextBlock second = first.next();
    QCOMPARE(title.begin().fragment().charFormat().fontPointSize(), 20.0 * 1.7);
    QVERIFY(Document::isTitle(title));
    QCOMPARE(Document::itemOf(title).title, QStringLiteral("Name"));
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
