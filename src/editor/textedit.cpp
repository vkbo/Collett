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
#include "textedit.h"

#include <QUuid>
#include <QObject>
#include <QWidget>
#include <QDateTime>
#include <QTextEdit>
#include <QJsonArray>
#include <QTextBlock>
#include <QJsonObject>
#include <QStringList>

namespace Collett {

GuiTextEdit::GuiTextEdit(QWidget *parent)
    : QTextEdit(parent)
{
}

/**
 * Methods
 * =======
 */

QJsonObject GuiTextEdit::toJsonObject() {

    QJsonObject json;
    QJsonArray jsonBlocks;

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
            case Qt::AlignHCenter:  jsonBlockFmt << "ac"; break;
            case Qt::AlignTrailing: jsonBlockFmt << "at"; break;
            case Qt::AlignJustify:  jsonBlockFmt << "aj"; break;
            default: break;
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

            jsonFrags.append(jsonFragFmt.join(":") + "|" + blockFrag.text());
        }

        qDebug() << jsonFrags.size();
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
        jsonBlocks.append(jsonBlock);
        block = block.next();
    }

    json.insert(QLatin1String("m:updated"), QDateTime::currentDateTime().toString(Qt::ISODate));
    json.insert(QLatin1String("x:content"), jsonBlocks);

    return json;
}

/**
 * Public Slots
 * ============
 */

void GuiTextEdit::applyDocAction(DocAction action) {

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

    } else if (action == Collett::TextAlignCentre) {
        setAlignment(Qt::AlignHCenter);

    } else if (action == Collett::TextAlignRight) {
        setAlignment(Qt::AlignRight);

    } else if (action == Collett::TextAlignJustify) {
        setAlignment(Qt::AlignJustify);

    } else if (action == Collett::TextIndent) {
        // Indenting is only allowed on text paragraphs (no heading level) that
        // are also aligned to the leading edge.
        QTextCursor cursor = textCursor();
        QTextBlockFormat format = cursor.blockFormat();
        if (format.headingLevel() == 0 && format.alignment() == Qt::AlignLeading) {
            format.setTextIndent(8.0);
            cursor.setBlockFormat(format);
        }

    } else if (action == Collett::TextOutdent) {
        // Text outdent is always allowed as there is no need to restrict it.
        QTextCursor cursor = textCursor();
        QTextBlockFormat format = cursor.blockFormat();
        format.setTextIndent(0.0);
        cursor.setBlockFormat(format);
    }
}

} // namespace Collett
