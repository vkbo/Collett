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
#include "settings.h"
#include "tools.h"

#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QObject>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>

using namespace Qt::Literals::StringLiterals;

namespace Collett {

// Constructor/Destructor
// ======================

Document::Document(QObject *parent) : QTextDocument(parent)
{
    m_createdTime = QDateTime::currentDateTime().toString(Qt::ISODate);
}

Document::~Document()
{
    qDebug() << "Destructor: Document";
}

// Public Methods
// ==============

void Document::pack(QJsonObject &data)
{

    QJsonArray jDoc;

    if (this->blockCount() == 1 && this->firstBlock().text().trimmed().isEmpty()) {
        // No text content
        return;
    }

    QTextBlock block = this->firstBlock();
    while (block.isValid()) {
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
        } else if (blockFormat.textIndent() < 0.0) {
            jsonBlockFmt << "sg";
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
        jDoc.append(jsonBlock);
        block = block.next();
    }

    // Populate Object
    QJsonObject jMeta;
    jMeta["m:created"_L1] = m_createdTime;
    jMeta["m:updated"_L1] = QDateTime::currentDateTime().toString(Qt::ISODate);

    data["c:format"_L1] = "CollettDocument";
    data["c:meta"_L1] = jMeta;
    data["x:content"_L1] = jDoc;
}

void Document::unpack(const QJsonObject &data)
{

    // Init
    qint64 start = QDateTime::currentMSecsSinceEpoch();

    Settings *settings = Settings::instance();
    Settings::TextFormat format = settings->textFormat();

    // Meta
    QJsonObject jMeta = data.value("c:meta"_L1).toObject();
    QJsonArray jDoc = data.value("x:content"_L1).toArray();

    m_createdTime = JsonUtils::getJsonString(jMeta, "m:created"_L1, "Unknown");

    // Unpack Text
    QTextCursor cursor = QTextCursor(this);
    bool isFirst = true;

    this->setUndoRedoEnabled(false);
    this->clear();

    for (const QJsonValue &jsonBlockValue : jDoc) {

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
            } else if (blockFmtTag == "sg") {
                blockFormat.setTextIndent(-format.tabWidth);
                blockFormat.setLeftMargin(format.tabWidth);
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

    this->setUndoRedoEnabled(true);
    this->setModified(false);

    qint64 end = QDateTime::currentMSecsSinceEpoch();
    qDebug() << "Document loaded in" << end - start << "ms";
}

} // namespace Collett
