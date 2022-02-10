/*
** Collett – GUI Text Editor Class
** ===============================
**
** This file is a part of Collett
** Copyright 2020–2022, Veronica Berglyd Olsen
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
#include "textedit.h"

#include <algorithm>

#include <QFont>
#include <QObject>
#include <QWidget>
#include <QDateTime>
#include <QTextEdit>
#include <QJsonArray>
#include <QJsonValue>
#include <QTextBlock>
#include <QJsonObject>
#include <QStringList>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QTextBlockFormat>

namespace Collett {

GuiTextEdit::GuiTextEdit(QWidget *parent)
    : QTextEdit(parent)
{
    // Settings
    setAcceptRichText(true);
    initDocument(this->document());

    CollettSettings *settings = CollettSettings::instance();
    m_format = settings->textFormat();

    connect(this, SIGNAL(cursorPositionChanged()),
            this, SLOT(processCursorPositionChanged()));
}

/**
 * Class Setters
 * =============
 */

void GuiTextEdit::setModified(bool state) {
    this->document()->setModified(state);
}

/**
 * Class Getters
 * =============
 */

bool GuiTextEdit::isModified() const {
    return this->document()->isModified();
}

/**
 * Class Methods
 * =============
 */

QJsonArray GuiTextEdit::toJsonContent() {

    QJsonArray json;

    if (this->document()->blockCount() == 1 && this->document()->firstBlock().text().trimmed().isEmpty()) {
        // No text content
        return json;
    }

    QTextBlock block = this->document()->firstBlock();
    while(block.isValid()) {
        QJsonObject jsonBlock;
        QJsonArray jsonFrags;
        QStringList jsonBlockFmt;

        QTextBlockFormat blockFormat = block.blockFormat();

        // Block Type
        if (blockFormat.headingLevel() > 0) {
            jsonBlockFmt << QString().setNum(blockFormat.headingLevel()).prepend("h");
        } else {
            jsonBlockFmt << "p";
        }

        // Block Alignment
        switch (blockFormat.alignment()) {
            case Qt::AlignLeading:  jsonBlockFmt << "al"; break;
            case Qt::AlignCenter:   jsonBlockFmt << "ac"; break;
            case Qt::AlignHCenter:  jsonBlockFmt << "ac"; break;
            case Qt::AlignTrailing: jsonBlockFmt << "at"; break;
            case Qt::AlignJustify:  jsonBlockFmt << "aj"; break;
            default: jsonBlockFmt << "al"; break;
        }

        // Text Indent
        if (blockFormat.textIndent() > 0.0) {
            jsonBlockFmt << "ti";
        } else if (blockFormat.textIndent() < 0.0) {
            jsonBlockFmt << "sg";
        }

        // Block Indent
        if (blockFormat.indent() > 0) {
            jsonBlockFmt << QString().setNum(blockFormat.indent()).prepend("in");
        }

        // Write Format
        jsonBlock.insert(QLatin1String("u:fmt"), jsonBlockFmt.join(":"));

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
            jsonBlock.insert(QLatin1String("u:txt"), "t|");
            break;
        case 1:
            jsonBlock.insert(QLatin1String("u:txt"), jsonFrags.at(0));
            break;
        default:
            jsonBlock.insert(QLatin1String("x:txt"), jsonFrags);
            break;
        }

        // Finish & Next
        json.append(jsonBlock);
        block = block.next();
    }

    return json;
}

void GuiTextEdit::setJsonContent(const QJsonArray &json) {

    QTextDocument *doc = new QTextDocument(this);
    QTextCursor cursor = QTextCursor(doc);
    bool isFirst = true;

    doc->setUndoRedoEnabled(false);
    doc->clear();
    initDocument(doc);

    for (const QJsonValue &jsonBlockValue : json) {

        if (!jsonBlockValue.isObject()) {
            qWarning() << "Unexpected content in JSON array. Expected JSON object.";
            continue;
        }

        QStringList jsonBlockFmt;
        QStringList jsonFrags;
        QTextBlock newBlock;

        QJsonObject jsonBlock = jsonBlockValue.toObject();
        if (jsonBlock.contains(QLatin1String("u:fmt"))) {
            jsonBlockFmt = jsonBlock[QLatin1String("u:fmt")].toString().split(":");
        }

        QTextCharFormat  charFormat = m_format.charDefault;
        QTextBlockFormat blockFormat = m_format.blockDefault;

        // The first block format entry must describe the block type
        if (!jsonBlockFmt.isEmpty()) {
            QString blockFmtType = jsonBlockFmt.first();
            if (blockFmtType == "p") {
                charFormat = m_format.charParagraph;
                blockFormat = m_format.blockParagraph;
            } else if (blockFmtType == "h1") {
                charFormat = m_format.charHeader1;
                blockFormat = m_format.blockHeader1;
            } else if (blockFmtType == "h2") {
                charFormat = m_format.charHeader2;
                blockFormat = m_format.blockHeader2;
            } else if (blockFmtType == "h3") {
                charFormat = m_format.charHeader3;
                blockFormat = m_format.blockHeader3;
            } else if (blockFmtType == "h4") {
                charFormat = m_format.charHeader3;
                blockFormat = m_format.blockHeader4;
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
                blockFormat.setTextIndent(m_format.tabWidth);
            } else if (blockFmtTag == "sg") {
                blockFormat.setTextIndent(-m_format.tabWidth);
                blockFormat.setLeftMargin(m_format.tabWidth);
            } else if (blockFmtTag.startsWith("in")) {
                blockFormat.setIndent(blockFmtTag.last(1).toInt());
            }
        }

        if (isFirst) {
            cursor.setBlockFormat(blockFormat);
            isFirst = false;
        } else {
            cursor.insertBlock(blockFormat);
        }

        if (jsonBlock.contains(QLatin1String("u:txt"))) {
            jsonFrags << jsonBlock[QLatin1String("u:txt")].toString();
        } else if (jsonBlock.contains(QLatin1String("x:txt"))) {
            for (const QJsonValue &jsonFragValue : jsonBlock[QLatin1String("x:txt")].toArray()) {
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
            bool isText = false;;
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

    doc->setUndoRedoEnabled(true);
    doc->setModified(false);

    this->setDocument(doc);
}

/**
 * Internal Functions
 * ==================
 */

void GuiTextEdit::initDocument(QTextDocument *doc) {

    // Text Options
    QTextOption opts;
    opts.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    doc->setDefaultTextOption(opts);
    doc->setDocumentMargin(40);

    // Editor Options
    this->setTabStopDistance(m_format.tabWidth);
}

/**
 * Public Slots
 * ============
 */

void GuiTextEdit::toggleBoldFormat() {
    if (this->fontWeight() > QFont::Medium) {
        this->setFontWeight(QFont::Normal);
    } else {
        this->setFontWeight(QFont::Bold);
    }
}

void GuiTextEdit::toggleItalicFormat() {
    this->setFontItalic(!this->fontItalic());
}

void GuiTextEdit::toggleUnderlineFormat() {
    this->setFontUnderline(!this->fontUnderline());
}

void GuiTextEdit::toggleStrikeOutFormat() {
    QFont font = this->currentFont();
    font.setStrikeOut(!font.strikeOut());
    this->setCurrentFont(font);
}

void GuiTextEdit::toggleSuperScriptFormat() {
    QTextCharFormat fmt = this->currentCharFormat();
    if (fmt.verticalAlignment() == QTextCharFormat::AlignSuperScript) {
        fmt.setVerticalAlignment(QTextCharFormat::AlignNormal);
    } else {
        fmt.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
    }
    this->setCurrentCharFormat(fmt);
}

void GuiTextEdit::toggleSubScriptFormat() {
    QTextCharFormat fmt = this->currentCharFormat();
    if (fmt.verticalAlignment() == QTextCharFormat::AlignSubScript) {
        fmt.setVerticalAlignment(QTextCharFormat::AlignNormal);
    } else {
        fmt.setVerticalAlignment(QTextCharFormat::AlignSubScript);
    }
    this->setCurrentCharFormat(fmt);
}

void GuiTextEdit::toggleSegmentFormat() {
    QTextCursor cursor = textCursor();
    QTextBlockFormat format = cursor.blockFormat();
    if (format.headingLevel() == 0 && format.alignment() == Qt::AlignLeading) {
        if (format.textIndent() < 0.0) {
            format.setTextIndent(0.0);
            format.setLeftMargin(0.0);
        } else {
            format.setTextIndent(-m_format.tabWidth);
            format.setLeftMargin(m_format.tabWidth);
        }
        cursor.setBlockFormat(format);
        emit currentBlockChanged(cursor.block());
    }
}

void GuiTextEdit::toggleFirstLineIndent() {
    QTextCursor cursor = textCursor();
    QTextBlockFormat format = cursor.blockFormat();
    if (format.headingLevel() == 0 && format.alignment() == Qt::AlignLeading) {
        if (format.textIndent() > 0.0) {
            format.setTextIndent(0.0);
            format.setLeftMargin(0.0);
        } else {
            format.setTextIndent(m_format.tabWidth);
            format.setLeftMargin(0.0);
        }
        cursor.setBlockFormat(format);
        emit currentBlockChanged(cursor.block());
    }
}

void GuiTextEdit::increaseBlockIndent() {
    QTextCursor cursor = textCursor();
    QTextBlockFormat format = cursor.blockFormat();
    if (format.headingLevel() == 0 && format.alignment() == Qt::AlignLeading) {
        format.setIndent(std::min(format.indent() + 1, 9));
        cursor.setBlockFormat(format);
        emit currentBlockChanged(cursor.block());
    }
}

void GuiTextEdit::decreaseBlockIndent() {
    QTextCursor cursor = textCursor();
    QTextBlockFormat format = cursor.blockFormat();
    if (format.headingLevel() == 0 && format.alignment() == Qt::AlignLeading) {
        format.setIndent(std::max(format.indent() - 1, 0));
        cursor.setBlockFormat(format);
        emit currentBlockChanged(cursor.block());
    }
}

void GuiTextEdit::applyBlockAlignment(const Qt::Alignment align) {
    this->setAlignment(align);
    QTextCursor cursor = this->textCursor();
    emit currentBlockChanged(cursor.block());
}

void GuiTextEdit::applyBlockFormat(BlockFormat format, int hLevel) {

    QTextCursor cursor = textCursor();
    int bPos = cursor.block().position();
    int bLen = cursor.block().length();
    int hLevelNow = cursor.blockFormat().headingLevel();

    if (bLen < 2) return;
    if (hLevel > 0 && hLevelNow == hLevel) return;

    cursor.beginEditBlock();
    cursor.setPosition(bPos, QTextCursor::MoveAnchor);
    cursor.setPosition(bPos + bLen - 1, QTextCursor::KeepAnchor);
    if (format == BlockFormat::Paragraph && hLevelNow != 0) {
        cursor.setBlockFormat(m_format.blockParagraph);
        cursor.setCharFormat(m_format.charParagraph);
    } else if (format == BlockFormat::Header && hLevel == 1) {
        cursor.setBlockFormat(m_format.blockHeader1);
        cursor.setCharFormat(m_format.charHeader1);
    } else if (format == BlockFormat::Header && hLevel == 2) {
        cursor.setBlockFormat(m_format.blockHeader2);
        cursor.setCharFormat(m_format.charHeader2);
    } else if (format == BlockFormat::Header && hLevel == 3) {
        cursor.setBlockFormat(m_format.blockHeader3);
        cursor.setCharFormat(m_format.charHeader3);
    } else if (format == BlockFormat::Header && hLevel == 4) {
        cursor.setBlockFormat(m_format.blockHeader4);
        cursor.setCharFormat(m_format.charHeader4);
    }
    cursor.endEditBlock();

    emit currentBlockChanged(cursor.block());
}

/**
 * Private Slots
 * =============
 */

void GuiTextEdit::processCursorPositionChanged() {

    QTextCursor cursor = textCursor();
    int blockNo = cursor.blockNumber();

    if (blockNo != m_currentBlockNo) {
        emit currentBlockChanged(cursor.block());
        m_currentBlockNo = blockNo;
    }
}

} // namespace Collett
