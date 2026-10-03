/*
** Collett - Document Class
** ========================
**
** This file is a part of Collett
** Copyright (C) 2025 Veronica Berglyd Olsen
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
#include "document.h"
#include "fonts.h"
#include "settings.h"

#include <QDateTime>
#include <QFontMetricsF>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QSet>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentFragment>

using namespace Qt::Literals::StringLiterals;

namespace Collett {

// Constructor/Destructor
// ======================

/**! @brief Create an empty document.
 *
 * The document follows the text format settings, so a font change in the
 * preferences is applied to the open documents.
 */
Document::Document(QObject *parent) : QTextDocument(parent)
{
    Settings *settings = Settings::instance();
    this->setDefaultFont(settings->textFont());
    connect(settings, &Settings::textFormatChanged, this, &Document::refreshTextFormat);
}

Document::~Document() {}

// Load and Save
// =============

/**! @brief Add a document at the end, with its title and text, as read
 * from a file.
 *
 * Nothing is added to the undo history, and the document is not modified.
 */
void Document::appendItem(const Item &item, const QJsonArray &content)
{
    const bool undo = this->isUndoRedoEnabled();
    this->setUndoRedoEnabled(false);

    QTextCursor cursor(this);
    cursor.movePosition(QTextCursor::End);
    if (isTitle(this->firstBlock())) {
        cursor.insertBlock();
    }
    setTitleFormat(cursor, item);
    cursor.insertText(item.title, titleCharFormat(item.level));
    appendContent(cursor, content);

    this->setUndoRedoEnabled(undo);
    this->setModified(false);
}

/**! @brief The text of a document, in the format of the files.
 *
 * A document with no text, or a single empty paragraph, has no content.
 */
QJsonArray Document::packContent(const QString &handle) const
{
    const QTextBlock title = titleBlock(handle);
    if (!title.isValid()) return QJsonArray();

    const QTextBlock first = title.next();
    const bool empty = !first.isValid() || isTitle(first) || ((!first.next().isValid() || isTitle(first.next())) && first.text().trimmed().isEmpty());
    return empty ? QJsonArray() : packBlocks(first);
}

// Structure
// =========

/**! @brief The documents, in reading order, as given by the title blocks.
 *
 * A title block with the handle of one before it is left out.
 */
QList<Document::Item> Document::items() const
{
    QList<Item> found;
    QSet<QString> handles;
    for (QTextBlock block = this->begin(); block.isValid(); block = block.next()) {
        if (!isTitle(block)) continue;
        Item item = itemOf(block);
        if (handles.contains(item.handle)) continue;
        handles.insert(item.handle);
        found.append(item);
    }
    return found;
}

/**! @brief The title block of a document, or an invalid block.
 */
QTextBlock Document::titleBlock(const QString &handle) const
{
    for (QTextBlock block = this->begin(); block.isValid(); block = block.next()) {
        if (isTitle(block) && block.blockFormat().stringProperty(HandleProperty) == handle) return block;
    }
    return QTextBlock();
}

/**! @brief The handle of the document a position is in.
 */
QString Document::handleAt(int position) const
{
    for (QTextBlock block = this->findBlock(position); block.isValid(); block = block.previous()) {
        if (isTitle(block)) return block.blockFormat().stringProperty(HandleProperty);
    }
    return QString();
}

/**! @brief The position at the end of the last block of a document, or -1.
 */
int Document::itemEnd(const QString &handle) const
{
    QTextBlock block = titleBlock(handle);
    if (!block.isValid()) return -1;
    while (block.next().isValid() && !isTitle(block.next())) {
        block = block.next();
    }
    return block.position() + block.length() - 1;
}

/**! @brief The text of a document, without its title, as plain text with
 * one line per paragraph.
 */
QString Document::itemText(const QString &handle) const
{
    QStringList lines;
    const QTextBlock title = titleBlock(handle);
    for (QTextBlock block = title.next(); title.isValid() && block.isValid() && !isTitle(block); block = block.next()) {
        lines.append(block.text());
    }
    return lines.join(u'\n');
}

/**! @brief The text blocks of a document, without its title, for counting.
 */
CountBlockList Document::snapshotItem(const QString &handle) const
{
    CountBlockList blocks;
    const QTextBlock title = titleBlock(handle);
    for (QTextBlock block = title.next(); title.isValid() && block.isValid() && !isTitle(block); block = block.next()) {
        blocks.append(TextCounter::snapshotBlock(block));
    }
    return blocks;
}

// Edits
// =====

/**! @brief Start a new document at a position in the text of another.
 *
 * A paragraph is split in two at the position, and the new title goes
 * between the halves. At the start or end of a paragraph, the title goes
 * before or after it, and an empty paragraph is turned into the title.
 * Returns the position of the new title, or -1 if the position is in a
 * title.
 */
int Document::insertItem(int position, const Item &item)
{
    const QTextBlock block = this->findBlock(position);
    if (!block.isValid() || isTitle(block)) return -1;

    QTextCursor cursor(this);
    cursor.beginEditBlock();
    const int offset = position - block.position();
    const int length = block.length() - 1;
    if (length == 0) {
        cursor.setPosition(block.position());
    } else if (offset == 0) {
        cursor.setPosition(block.position() - 1);
        cursor.insertBlock();
    } else if (offset == length) {
        cursor.setPosition(position);
        cursor.insertBlock();
    } else {
        cursor.setPosition(position);
        cursor.insertBlock();
        cursor.setPosition(position);
        cursor.insertBlock();
    }
    setTitleFormat(cursor, item);
    cursor.insertText(item.title, titleCharFormat(item.level));
    cursor.endEditBlock();
    return cursor.block().position();
}

/**! @brief Remove a document, with its title and text.
 *
 * The last document cannot be removed.
 */
bool Document::removeItem(const QString &handle)
{
    const QTextBlock title = titleBlock(handle);
    if (!title.isValid() || items().size() <= 1) return false;

    QTextBlock next = title.next();
    while (next.isValid() && !isTitle(next)) {
        next = next.next();
    }

    // Whole blocks are removed, so the blocks around keep their formats
    QTextCursor cursor(this);
    if (next.isValid()) {
        cursor.setPosition(title.position());
        cursor.setPosition(next.position(), QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(title.position() - 1);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    }
    cursor.removeSelectedText();
    return true;
}

/**! @brief Merge a document into the one before it, by removing its title.
 *
 * The text follows the text of the previous document. Returns the position
 * where the merged text starts, or -1 if there is no document before.
 */
int Document::mergeItem(const QString &handle)
{
    const QTextBlock title = titleBlock(handle);
    if (!title.isValid() || title.position() == 0) return -1;

    QTextCursor cursor(this);
    const QTextBlock next = title.next();
    if (next.isValid() && !isTitle(next)) {
        const int start = title.position();
        cursor.setPosition(start);
        cursor.setPosition(next.position(), QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        return start;
    }
    cursor.setPosition(title.position() - 1);
    cursor.setPosition(title.position() + title.length() - 1, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    return cursor.position();
}

/**! @brief Replace the title text of a document.
 */
bool Document::setItemTitle(const QString &handle, const QString &title)
{
    const QTextBlock block = titleBlock(handle);
    if (!block.isValid() || block.text() == title) return false;

    QTextCursor cursor(this);
    cursor.setPosition(block.position());
    cursor.setPosition(block.position() + block.length() - 1, QTextCursor::KeepAnchor);
    cursor.insertText(title, titleCharFormat(itemOf(block).level));
    return true;
}

/**! @brief Set the level and break settings of a document. The title text
 * is not changed, but is restyled for the level.
 */
bool Document::setItemValues(const Item &item)
{
    const QTextBlock block = titleBlock(item.handle);
    if (!block.isValid()) return false;

    const Item old = itemOf(block);
    if (old.level == item.level && old.hardBreak == item.hardBreak && old.numbered == item.numbered) return false;

    Item values = item;
    values.title = old.title;
    QTextCursor cursor(this);
    cursor.setPosition(block.position());
    setTitleFormat(cursor, values);
    return true;
}

/**! @brief Move count documents from row to before another row, as one
 * edit.
 *
 * The documents are cut out, and pasted into an empty block made at the
 * target, as a pasted block takes the format of the block it is pasted
 * into. The format of the first title is then put back.
 */
bool Document::moveItems(int row, int count, int before)
{
    const QList<Item> list = items();
    const int total = int(list.size());
    if (count < 1 || row < 0 || row + count > total || before < 0 || before > total || (before >= row && before <= row + count)) {
        return false;
    }

    const QTextBlock first = titleBlock(list.at(row).handle);
    const QTextBlockFormat firstFormat = first.blockFormat();
    const QTextCharFormat firstCharFormat = first.charFormat();
    const QString target = before < total ? list.at(before).handle : QString();

    QTextCursor cursor(this);
    cursor.beginEditBlock();

    // Cut the documents out as whole blocks
    cursor.setPosition(first.position());
    cursor.setPosition(itemEnd(list.at(row + count - 1).handle), QTextCursor::KeepAnchor);
    const QTextDocumentFragment moved = cursor.selection();
    if (row + count < total) {
        cursor.setPosition(first.position());
        cursor.setPosition(titleBlock(list.at(row + count).handle).position(), QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(first.position() - 1);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    }
    cursor.removeSelectedText();

    // Make an empty block where they go
    const QTextBlock targetBlock = titleBlock(target);
    if (!targetBlock.isValid()) {
        cursor.movePosition(QTextCursor::End);
        cursor.insertBlock();
    } else if (targetBlock.position() == 0) {
        cursor.setPosition(0);
        cursor.insertBlock();
        cursor.setPosition(0);
    } else {
        cursor.setPosition(targetBlock.position() - 1);
        cursor.insertBlock();
    }

    const int start = cursor.position();
    cursor.insertFragment(moved);
    cursor.setPosition(start);
    cursor.setBlockFormat(firstFormat);
    cursor.setBlockCharFormat(firstCharFormat);
    cursor.endEditBlock();
    return true;
}

/**! @brief Fix up the structure after an edit, as part of the same undo step.
 *
 * Text before the first title, which is left when the first title is
 * removed along with text after it, gets a new empty title with the given
 * handle. A title block with the handle of one before it, which is made by
 * a line break typed or pasted in a title, becomes a paragraph. Returns true
 * if anything was changed.
 */
bool Document::normalize(const QString &newHandle)
{
    const bool needsTitle = !isTitle(this->firstBlock());
    QList<int> duplicates;
    QSet<QString> handles;
    for (QTextBlock block = this->begin(); block.isValid(); block = block.next()) {
        if (!isTitle(block)) continue;
        const QString handle = block.blockFormat().stringProperty(HandleProperty);
        if (handles.contains(handle)) duplicates.append(block.position());
        handles.insert(handle);
    }
    if (!needsTitle && duplicates.isEmpty()) return false;

    const Settings::TextFormat format = Settings::instance()->textFormat();
    QTextCursor cursor(this);
    cursor.joinPreviousEditBlock();
    for (const int position : std::as_const(duplicates)) {
        const QTextBlock block = this->findBlock(position);
        cursor.setPosition(block.position());
        cursor.setBlockFormat(format.blockParagraph);
        cursor.setBlockCharFormat(format.charParagraph);
        cursor.setPosition(block.position() + block.length() - 1, QTextCursor::KeepAnchor);
        cursor.setCharFormat(format.charParagraph);
    }
    if (needsTitle) {
        Item item;
        item.handle = newHandle;
        cursor.setPosition(0);
        cursor.insertBlock();
        cursor.setPosition(0);
        setTitleFormat(cursor, item);
    }
    cursor.endEditBlock();
    return true;
}

/**! @brief Make the blocks from position first to last paragraphs, with
 * level 0, or headings of level 1 to 4.
 *
 * The blocks keep their alignment and indent, and their text keeps its
 * italic and other flags. The font family, size and weight come from the
 * new type. Headings have no first-line indent. Title blocks are left as
 * they are.
 */
void Document::setHeadingLevel(int first, int last, int level)
{
    const Settings::TextFormat format = Settings::instance()->textFormat();
    QTextBlockFormat baseBlock = format.blockParagraph;
    QTextCharFormat baseChar = format.charParagraph;
    switch (level) {
    case 1:
        baseBlock = format.blockHeader1;
        baseChar = format.charHeader1;
        break;
    case 2:
        baseBlock = format.blockHeader2;
        baseChar = format.charHeader2;
        break;
    case 3:
        baseBlock = format.blockHeader3;
        baseChar = format.charHeader3;
        break;
    case 4:
        baseBlock = format.blockHeader4;
        baseChar = format.charHeader4;
        break;
    default:
        break;
    }

    QTextCharFormat fontFormat;
    fontFormat.setFontFamilies(baseChar.fontFamilies().toStringList());
    fontFormat.setFontPointSize(baseChar.fontPointSize());
    fontFormat.setFontWeight(baseChar.fontWeight());

    QTextCursor cursor(this);
    cursor.beginEditBlock();
    const QTextBlock end = this->findBlock(last);
    for (QTextBlock block = this->findBlock(first); block.isValid(); block = block.next()) {
        if (isTitle(block)) {
            if (block == end) break;
            continue;
        }
        const QTextBlockFormat oldFormat = block.blockFormat();
        QTextBlockFormat blockFormat = baseBlock;
        blockFormat.setAlignment(oldFormat.alignment());
        blockFormat.setIndent(oldFormat.indent());
        if (level == 0 && oldFormat.textIndent() > 0.0) {
            blockFormat.setTextIndent(format.tabWidth);
        }

        cursor.setPosition(block.position());
        cursor.setBlockFormat(blockFormat);
        cursor.mergeBlockCharFormat(fontFormat);
        cursor.setPosition(block.position() + block.length() - 1, QTextCursor::KeepAnchor);
        cursor.mergeCharFormat(fontFormat);
        if (block == end) break;
    }
    cursor.endEditBlock();
}

// Static Methods
// ==============

/**! @brief Check if a block is the title of a document.
 */
bool Document::isTitle(const QTextBlock &block)
{
    return block.isValid() && block.blockFormat().hasProperty(HandleProperty);
}

/**! @brief The values of a document, read from its title block.
 */
Document::Item Document::itemOf(const QTextBlock &block)
{
    const QTextBlockFormat format = block.blockFormat();
    Item item;
    item.handle = format.stringProperty(HandleProperty);
    item.title = block.text();
    item.level = ItemLevel(qBound(int(ItemLevel::PartitionLevel), format.intProperty(LevelProperty), int(ItemLevel::PageLevel)));
    item.hardBreak = format.boolProperty(HardBreakProperty);
    item.numbered = !format.hasProperty(NumberedProperty) || format.boolProperty(NumberedProperty);
    return item;
}

/**! @brief The block format of a title.
 *
 * The space above the title holds the divider from the document before,
 * and the label with the level, which are drawn by the editor.
 */
QTextBlockFormat Document::titleBlockFormat(const Item &item)
{
    QTextBlockFormat format = Settings::instance()->textFormat().blockDefault;
    format.setProperty(HandleProperty, item.handle);
    format.setProperty(LevelProperty, int(item.level));
    format.setProperty(HardBreakProperty, item.hardBreak);
    format.setProperty(NumberedProperty, item.numbered);
    format.setTopMargin(dividerHeight(item.level, item.hardBreak) + labelHeight() + 4.0);
    format.setBottomMargin(12.0);
    return format;
}

/**! @brief The character format of a title, which is the heading font,
 * larger for partitions and chapters.
 */
QTextCharFormat Document::titleCharFormat(ItemLevel level)
{
    qreal factor = 1.4;
    if (level == ItemLevel::PartitionLevel) {
        factor = 2.0;
    } else if (level == ItemLevel::ChapterLevel) {
        factor = 1.7;
    }
    QTextCharFormat format;
    format.setFont(Fonts::scaled(Settings::instance()->headingFont(), factor));
    return format;
}

/**! @brief The space between a document and the one before it.
 *
 * Partitions and chapters get the space of three empty paragraphs, and
 * other documents the space of one. A scene with a hard break gets three,
 * with the break drawn in the middle one.
 */
qreal Document::dividerHeight(ItemLevel level, bool hardBreak)
{
    const QFont font = Settings::instance()->textFont();
    const qreal paragraph = QFontMetricsF(font).height() * 1.15 + font.pointSizeF();
    const bool major = level == ItemLevel::PartitionLevel || level == ItemLevel::ChapterLevel;
    return (major || (hardBreak && level == ItemLevel::SceneLevel) ? 3 : 1) * paragraph;
}

/**! @brief The height of the level label above a title.
 */
qreal Document::labelHeight()
{
    return QFontMetricsF(Settings::instance()->headingFont()).height();
}

// Public Slots
// ============

/**! @brief Reapply the font and spacing from the current text format settings.
 *
 * Each block keeps its type, alignment and indent, and each fragment keeps
 * its bold, italic and other flags. Titles are restyled for their level. Only the font family and size, and the
 * margins that depend on the size, are replaced. The undo history is
 * cleared, since a settings change is not an edit, and the modified state
 * is left as it was.
 */
void Document::refreshTextFormat()
{
    Settings *settings = Settings::instance();
    const Settings::TextFormat format = settings->textFormat();
    const bool wasModified = this->isModified();

    this->setDefaultFont(settings->textFont());
    this->setUndoRedoEnabled(false);

    QTextCursor cursor(this);
    for (QTextBlock block = this->begin(); block.isValid(); block = block.next()) {
        if (isTitle(block)) {
            cursor.setPosition(block.position());
            setTitleFormat(cursor, itemOf(block));
            continue;
        }
        QTextBlockFormat blockFormat = block.blockFormat();
        QTextBlockFormat baseBlock = format.blockParagraph;
        QTextCharFormat baseChar = format.charParagraph;
        switch (blockFormat.headingLevel()) {
        case 1:
            baseBlock = format.blockHeader1;
            baseChar = format.charHeader1;
            break;
        case 2:
            baseBlock = format.blockHeader2;
            baseChar = format.charHeader2;
            break;
        case 3:
            baseBlock = format.blockHeader3;
            baseChar = format.charHeader3;
            break;
        case 4:
            baseBlock = format.blockHeader4;
            baseChar = format.charHeader4;
            break;
        default:
            break;
        }

        blockFormat.setTopMargin(baseBlock.topMargin());
        blockFormat.setBottomMargin(baseBlock.bottomMargin());
        blockFormat.setLineHeight(baseBlock.lineHeight(), baseBlock.lineHeightType());
        if (blockFormat.textIndent() > 0.0) {
            blockFormat.setTextIndent(format.tabWidth);
        }

        QTextCharFormat fontFormat;
        fontFormat.setFontFamilies(baseChar.fontFamilies().toStringList());
        fontFormat.setFontPointSize(baseChar.fontPointSize());

        cursor.setPosition(block.position());
        cursor.setBlockFormat(blockFormat);
        cursor.mergeBlockCharFormat(fontFormat);
        cursor.setPosition(block.position() + block.length() - 1, QTextCursor::KeepAnchor);
        cursor.mergeCharFormat(fontFormat);
    }

    this->setUndoRedoEnabled(true);
    this->setModified(wasModified);
}

// Private Methods
// ===============

/**! @brief Add paragraphs after the cursor, from the format of the files.
 */
void Document::appendContent(QTextCursor &cursor, const QJsonArray &content) const
{
    Settings::TextFormat format = Settings::instance()->textFormat();

    for (const QJsonValue &jsonBlockValue : content) {

        if (!jsonBlockValue.isObject()) {
            qWarning() << "Unexpected content in JSON array. Expected JSON object.";
            continue;
        }

        QStringList jsonBlockFmt;
        QStringList jsonFrags;
        QTextBlock newBlock;

        QJsonObject jsonBlock = jsonBlockValue.toObject();
        if (jsonBlock.contains("u:fmt"_L1)) {
            jsonBlockFmt = jsonBlock["u:fmt"_L1].toString().split(":");
        }

        QTextCharFormat charFormat = format.charDefault;
        QTextBlockFormat blockFormat = format.blockDefault;

        // The first block format entry must describe the block type
        if (!jsonBlockFmt.isEmpty()) {
            QString blockFmtType = jsonBlockFmt.first();
            if (blockFmtType == "p") {
                charFormat = format.charParagraph;
                blockFormat = format.blockParagraph;
            } else if (blockFmtType == "h1") {
                charFormat = format.charHeader1;
                blockFormat = format.blockHeader1;
            } else if (blockFmtType == "h2") {
                charFormat = format.charHeader2;
                blockFormat = format.blockHeader2;
            } else if (blockFmtType == "h3") {
                charFormat = format.charHeader3;
                blockFormat = format.blockHeader3;
            } else if (blockFmtType == "h4") {
                charFormat = format.charHeader4;
                blockFormat = format.blockHeader4;
            }
            jsonBlockFmt.removeFirst();
        }

        // The remaining block format entries describe the other format flags
        for (const QString &blockFmtTag : jsonBlockFmt) {
            if (blockFmtTag == "al") {
                blockFormat.setAlignment(Qt::AlignLeading);
            } else if (blockFmtTag == "ac") {
                blockFormat.setAlignment(Qt::AlignHCenter);
            } else if (blockFmtTag == "at") {
                blockFormat.setAlignment(Qt::AlignTrailing);
            } else if (blockFmtTag == "aj") {
                blockFormat.setAlignment(Qt::AlignJustify);
            } else if (blockFmtTag == "ti") {
                blockFormat.setTextIndent(format.tabWidth);
            } else if (blockFmtTag.startsWith("in")) {
                blockFormat.setIndent(qBound(0, blockFmtTag.sliced(2).toInt(), 9));
            }
        }

        cursor.insertBlock(blockFormat, charFormat);

        if (jsonBlock.contains("u:txt"_L1)) {
            jsonFrags << jsonBlock["u:txt"_L1].toString();
        } else if (jsonBlock.contains("x:txt"_L1)) {
            for (const QJsonValue &jsonFragValue : jsonBlock["x:txt"_L1].toArray()) {
                jsonFrags << jsonFragValue.toString();
            }
        }

        for (const QString &fragText : jsonFrags) {

            qsizetype fmtTagPos = fragText.indexOf("|");
            if (fmtTagPos < 0) {
                qWarning() << "Could not parse format of text line";
                cursor.insertText(fragText);
                continue;
            }

            QStringList fragCharFmt = fragText.first(fmtTagPos).split(":");
            QString innerText = fragText.sliced(fmtTagPos + 1).replace('\n', QChar::LineSeparator);

            QTextCharFormat fragFormat = charFormat;
            bool isText = false;
            for (const QString &fragFmtTag : fragCharFmt) {
                if (fragFmtTag == "t") {
                    isText = true;
                } else if (fragFmtTag == "b") {
                    fragFormat.setFontWeight(QFont::Bold);
                } else if (fragFmtTag == "i") {
                    fragFormat.setFontItalic(true);
                } else if (fragFmtTag == "u") {
                    fragFormat.setFontUnderline(true);
                } else if (fragFmtTag == "s") {
                    fragFormat.setFontStrikeOut(true);
                } else if (fragFmtTag == "sup") {
                    fragFormat.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
                } else if (fragFmtTag == "sub") {
                    fragFormat.setVerticalAlignment(QTextCharFormat::AlignSubScript);
                }
            }

            if (isText) {
                cursor.insertText(innerText, fragFormat);
            }
        }
    }
}

/**! @brief Pack the blocks from a block up to the next title, in the format
 * of the files.
 */
QJsonArray Document::packBlocks(QTextBlock block) const
{
    QJsonArray content;
    while (block.isValid() && !isTitle(block)) {
        QJsonObject jsonBlock;
        QJsonArray jsonFrags;
        QStringList jsonBlockFmt;

        QTextBlockFormat blockFormat = block.blockFormat();

        // Block Type
        if (blockFormat.headingLevel() > 0) {
            jsonBlockFmt << QString().setNum(qBound(1, blockFormat.headingLevel(), 4)).prepend("h");
        } else {
            jsonBlockFmt << "p";
        }

        // Block Alignment
        switch (blockFormat.alignment()) {
        case Qt::AlignLeading: jsonBlockFmt << "al"; break;
        case Qt::AlignCenter: jsonBlockFmt << "ac"; break;
        case Qt::AlignHCenter: jsonBlockFmt << "ac"; break;
        case Qt::AlignTrailing: jsonBlockFmt << "at"; break;
        case Qt::AlignJustify: jsonBlockFmt << "aj"; break;
        default: jsonBlockFmt << "al"; break;
        }

        // Text Indent
        if (blockFormat.textIndent() > 0.0) {
            jsonBlockFmt << "ti";
        }

        // Block Indent
        if (blockFormat.indent() > 0) {
            jsonBlockFmt << QString().setNum(blockFormat.indent()).prepend("in");
        }

        // Write Format
        jsonBlock.insert("u:fmt"_L1, jsonBlockFmt.join(":"));

        // Write Text
        QTextBlock::Iterator blockIt = block.begin();
        for (; !blockIt.atEnd(); ++blockIt) {

            QJsonObject jsonFrag;
            QTextFragment blockFrag = blockIt.fragment();
            QTextCharFormat fragFmt = blockFrag.charFormat();

            QStringList jsonFragFmt;

            jsonFragFmt << "t";
            if (fragFmt.fontWeight() > QFont::Medium) jsonFragFmt << "b";
            if (fragFmt.fontItalic()) jsonFragFmt << "i";
            if (fragFmt.fontUnderline()) jsonFragFmt << "u";
            if (fragFmt.fontStrikeOut()) jsonFragFmt << "s";
            if (fragFmt.verticalAlignment() == QTextCharFormat::AlignSuperScript) jsonFragFmt << "sup";
            if (fragFmt.verticalAlignment() == QTextCharFormat::AlignSubScript) jsonFragFmt << "sub";

            jsonFrags.append(jsonFragFmt.join(":") + "|" + blockFrag.text().replace(QChar::LineSeparator, '\n'));
        }

        switch (jsonFrags.size()) {
        case 0:
            jsonBlock.insert("u:txt"_L1, "t|");
            break;
        case 1:
            jsonBlock.insert("u:txt"_L1, jsonFrags.at(0));
            break;
        default:
            jsonBlock.insert("x:txt"_L1, jsonFrags);
            break;
        }

        // Finish & Next
        content.append(jsonBlock);
        block = block.next();
    }

    return content;
}

/**! @brief Make the block at the cursor a title, with the values of a
 * document. Its text is styled for the level.
 */
void Document::setTitleFormat(QTextCursor &cursor, const Item &item) const
{
    const QTextCharFormat charFormat = titleCharFormat(item.level);
    const QTextBlock block = cursor.block();
    cursor.beginEditBlock();
    cursor.setPosition(block.position());
    cursor.setBlockFormat(titleBlockFormat(item));
    cursor.setBlockCharFormat(charFormat);
    if (block.length() > 1) {
        cursor.setPosition(block.position() + block.length() - 1, QTextCursor::KeepAnchor);
        cursor.mergeCharFormat(charFormat);
        cursor.clearSelection();
    }
    cursor.endEditBlock();
}

} // namespace Collett
