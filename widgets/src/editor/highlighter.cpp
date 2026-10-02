/*
** Collett - Document Highlighter Class
** ====================================
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

#include "highlighter.h"
#include "textblock.h"
#include "theme.h"

#include <QFont>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextFragment>

namespace Collett {

// Constructor/Destructor
// ======================

GuiDocHighlighter::GuiDocHighlighter(QObject *parent) : QSyntaxHighlighter(parent)
{
    this->updateTheme();
}

GuiDocHighlighter::~GuiDocHighlighter()
{
    qDebug() << "Destructor: GuiDocHighlighter";
}

// Public Methods
// ==============

/**! @brief Rebuild the overlay formats from the current theme.
 *
 * The caller must rehighlight the document afterwards for the new colours
 * to show.
 */
void GuiDocHighlighter::updateTheme()
{
    Theme *theme = Theme::instance();

    m_fmtHeader = QTextCharFormat();
    m_fmtHeader.setForeground(theme->getSyntaxColor(SyntaxHeader));

    m_fmtEmphasis = QTextCharFormat();
    m_fmtEmphasis.setForeground(theme->getSyntaxColor(SyntaxEmphasis));

    m_fmtComment = QTextCharFormat();
    m_fmtComment.setForeground(theme->getSyntaxColor(SyntaxComment));
}

// Protected Methods
// =================

/**! @brief Colour a single block and snapshot its text.
 *
 * Comment blocks and headings are coloured as a whole. In other blocks, the
 * bold and italic fragments get the emphasis colour, unless the fragment has
 * a stored foreground colour of its own, which always wins over the theme.
 * Finally the block text is stored in the block data for the background
 * checks, which also clears any cached errors for the block.
 */
void GuiDocHighlighter::highlightBlock(const QString &text)
{
    QTextBlock block = this->currentBlock();
    QTextBlockFormat blockFormat = block.blockFormat();
    int length = text.length();

    if (blockFormat.intProperty(BlockTypeProperty) == CommentBlock) {
        this->setFormat(0, length, m_fmtComment);
    } else if (blockFormat.headingLevel() > 0) {
        this->setFormat(0, length, m_fmtHeader);
    } else {
        int blockPos = block.position();
        for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
            QTextFragment fragment = it.fragment();
            if (!fragment.isValid()) {
                continue;
            }
            QTextCharFormat charFormat = fragment.charFormat();
            if (charFormat.hasProperty(QTextFormat::ForegroundBrush)) {
                continue;
            }
            if (charFormat.fontWeight() > QFont::Medium || charFormat.fontItalic()) {
                this->setFormat(fragment.position() - blockPos, fragment.length(), m_fmtEmphasis);
            }
        }
    }

    TextBlockData *data = dynamic_cast<TextBlockData *>(this->currentBlockUserData());
    if (data == nullptr) {
        data = new TextBlockData();
        this->setCurrentBlockUserData(data);
    }
    data->processText(text);
}

} // namespace Collett
