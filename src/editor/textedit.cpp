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

    // Text Options
    QTextOption opts;
    opts.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    document()->setDefaultTextOption(opts);
    document()->setDocumentMargin(40);

    CollettSettings *settings = CollettSettings::instance();
    m_format = settings->textFormat();

    connect(this, SIGNAL(cursorPositionChanged()),
            this, SLOT(processCursorPositionChanged()));
}

/**
 * Methods
 * =======
 */

QJsonArray GuiTextEdit::toJsonContent() {

    QJsonArray json;

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

    QTextDocument *doc = this->document();
    QTextCursor cursor = QTextCursor(doc);
    bool isFirst = true;

    doc->setUndoRedoEnabled(false);
    doc->clear();

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
                blockFormat.setTextIndent(m_format.textIndent);
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
                }
            }

            if (isText) {
                cursor.insertText(innerText, fragFormat);
            }
        }

    }

    doc->setUndoRedoEnabled(true);
}

/**
 * Public Slots
 * ============
 */

void GuiTextEdit::applyDocAction(DocAction action) {

    bool blockChanged = false;

    if (action == Collett::FormatBold) {
        if (fontWeight() > QFont::Medium) {
            setFontWeight(QFont::Normal);
        } else {
            setFontWeight(QFont::Bold);
        }

    } else if (action == Collett::FormatItalic) {
        setFontItalic(!fontItalic());

    } else if (action == Collett::FormatUnderline) {
        setFontUnderline(!fontUnderline());

    } else if (action == Collett::FormatStrikethrough) {
        QFont font = currentFont();
        font.setStrikeOut(!font.strikeOut());
        setCurrentFont(font);

    } else if (action == Collett::TextAlignLeft) {
        setAlignment(Qt::AlignLeft);
        blockChanged = true;

    } else if (action == Collett::TextAlignCentre) {
        setAlignment(Qt::AlignHCenter);
        blockChanged = true;

    } else if (action == Collett::TextAlignRight) {
        setAlignment(Qt::AlignRight);
        blockChanged = true;

    } else if (action == Collett::TextAlignJustify) {
        setAlignment(Qt::AlignJustify);
        blockChanged = true;

    } else if (action == Collett::TextIndent) {
        QTextCursor cursor = textCursor();
        QTextBlockFormat format = cursor.blockFormat();
        if (format.headingLevel() == 0 && format.alignment() == Qt::AlignLeading) {
            if (format.textIndent() > 0.0) {
                format.setTextIndent(0.0);
            } else {
                format.setTextIndent(m_format.textIndent);
            }
            cursor.setBlockFormat(format);
        }
        blockChanged = true;

    } else if (action == Collett::BlockIndent) {
        QTextCursor cursor = textCursor();
        QTextBlockFormat format = cursor.blockFormat();
        if (format.headingLevel() == 0 && format.alignment() == Qt::AlignLeading) {
            format.setIndent(std::min(format.indent() + 1, 9));
            cursor.setBlockFormat(format);
        }
        blockChanged = true;

    } else if (action == Collett::BlockOutdent) {
        QTextCursor cursor = textCursor();
        QTextBlockFormat format = cursor.blockFormat();
        if (format.headingLevel() == 0 && format.alignment() == Qt::AlignLeading) {
            format.setIndent(std::max(format.indent() - 1, 0));
            cursor.setBlockFormat(format);
        }
        blockChanged = true;
    }

    if (blockChanged) {
        QTextCursor cursor = textCursor();
        emit currentBlockChanged(cursor.block());
    }
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
