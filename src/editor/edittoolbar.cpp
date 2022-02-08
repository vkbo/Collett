/*
** Collett – GUI Editor Tool Bar Class
** ===================================
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
#include "edittoolbar.h"
#include "icons.h"

#include <QMenu>
#include <QSize>
#include <QObject>
#include <QWidget>
#include <QToolBar>
#include <QSizePolicy>
#include <QActionGroup>

namespace Collett {

GuiEditToolBar::GuiEditToolBar(QWidget *parent)
    : QToolBar(parent)
{
    CollettIcons *icons = CollettIcons::instance();

    this->setIconSize(QSize(20, 20));

    // Text Block Format Menu
    // ======================

    m_formatTextMenu = new QMenu();

    m_formatTextGroup = new QActionGroup(this);
    m_formatTextGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);

    m_formatParagraph = m_formatTextMenu->addAction(tr("Paragraph"));
    m_formatParagraph->setCheckable(true);
    m_formatParagraph->setActionGroup(m_formatTextGroup);

    m_formatHeader1 = m_formatTextMenu->addAction(tr("Header 1"));
    m_formatHeader1->setCheckable(true);
    m_formatHeader1->setActionGroup(m_formatTextGroup);

    m_formatHeader2 = m_formatTextMenu->addAction(tr("Header 2"));
    m_formatHeader2->setCheckable(true);
    m_formatHeader2->setActionGroup(m_formatTextGroup);

    m_formatHeader3 = m_formatTextMenu->addAction(tr("Header 3"));
    m_formatHeader3->setCheckable(true);
    m_formatHeader3->setActionGroup(m_formatTextGroup);

    m_formatHeader4 = m_formatTextMenu->addAction(tr("Header 4"));
    m_formatHeader4->setCheckable(true);
    m_formatHeader4->setActionGroup(m_formatTextGroup);

    m_formatText = new QToolButton(this);
    m_formatText->setIcon(icons->icon("formatText"));
    m_formatText->setText(tr("Format"));
    m_formatText->setMenu(m_formatTextMenu);
    m_formatText->setPopupMode(QToolButton::InstantPopup);
    this->addWidget(m_formatText);

    connect(m_formatParagraph, &QAction::triggered, [this]{emitDocumentAction(DocAction::BlockParagraph);});
    connect(m_formatHeader1, &QAction::triggered, [this]{emitDocumentAction(DocAction::BlockHeader1);});
    connect(m_formatHeader2, &QAction::triggered, [this]{emitDocumentAction(DocAction::BlockHeader2);});
    connect(m_formatHeader3, &QAction::triggered, [this]{emitDocumentAction(DocAction::BlockHeader3);});
    connect(m_formatHeader4, &QAction::triggered, [this]{emitDocumentAction(DocAction::BlockHeader4);});

    this->addSeparator();

    // Character Format Buttons
    // ========================

    m_formatBold = this->addAction(icons->icon("bold"), tr("Bold"));
    m_formatBold->setCheckable(true);

    m_formatItalic = this->addAction(icons->icon("italic"), tr("Italic"));
    m_formatItalic->setCheckable(true);

    m_formatUnderline = this->addAction(icons->icon("underline"), tr("Underline"));
    m_formatUnderline->setCheckable(true);

    m_formatStrikethrough = this->addAction(icons->icon("strikethrough"), tr("Strikethrough"));
    m_formatStrikethrough->setCheckable(true);

    connect(m_formatBold, &QAction::triggered, [this]{emitDocumentAction(DocAction::FormatBold);});
    connect(m_formatItalic, &QAction::triggered, [this]{emitDocumentAction(DocAction::FormatItalic);});
    connect(m_formatUnderline, &QAction::triggered, [this]{emitDocumentAction(DocAction::FormatUnderline);});
    connect(m_formatStrikethrough, &QAction::triggered, [this]{emitDocumentAction(DocAction::FormatStrikethrough);});

    this->addSeparator();

    m_formatSubscript = this->addAction(icons->icon("subscript"), tr("Subscript"));
    m_formatSubscript->setCheckable(true);

    m_formatSuperscript = this->addAction(icons->icon("superscript"), tr("Superscript"));
    m_formatSuperscript->setCheckable(true);

    connect(m_formatSubscript, &QAction::triggered, [this]{emitDocumentAction(DocAction::FormatSubscript);});
    connect(m_formatSuperscript, &QAction::triggered, [this]{emitDocumentAction(DocAction::FormatSuperscript);});

    this->addSeparator();

    // Text Block Alignment Options
    // ============================

    m_alignTextGroup = new QActionGroup(this);
    m_alignTextGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);

    m_alignLeft = this->addAction(icons->icon("alignLeft"), tr("Align Left"));
    m_alignLeft->setCheckable(true);
    m_alignLeft->setActionGroup(m_alignTextGroup);

    m_alignCentre = this->addAction(icons->icon("alignCentre"), tr("Align Right"));
    m_alignCentre->setCheckable(true);
    m_alignCentre->setActionGroup(m_alignTextGroup);

    m_alignRight = this->addAction(icons->icon("alignRight"), tr("Align Centre"));
    m_alignRight->setCheckable(true);
    m_alignRight->setActionGroup(m_alignTextGroup);

    m_alignJustify = this->addAction(icons->icon("alignJustify"), tr("Align Justify"));
    m_alignJustify->setCheckable(true);
    m_alignJustify->setActionGroup(m_alignTextGroup);

    connect(m_alignLeft, &QAction::triggered, [this]{emitDocumentAction(DocAction::TextAlignLeft);});
    connect(m_alignCentre, &QAction::triggered, [this]{emitDocumentAction(DocAction::TextAlignCentre);});
    connect(m_alignRight, &QAction::triggered, [this]{emitDocumentAction(DocAction::TextAlignRight);});
    connect(m_alignJustify, &QAction::triggered, [this]{emitDocumentAction(DocAction::TextAlignJustify);});

    this->addSeparator();

    // Text Block Indentation and Style
    // ================================

    m_formatStyleGroup = new QActionGroup(this);
    m_formatStyleGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);

    m_textQuote = this->addAction(icons->icon("quote"), tr("Block Quote"));
    m_textQuote->setCheckable(true);
    m_textQuote->setActionGroup(m_formatStyleGroup);

    m_textSegment = this->addAction(icons->icon("textSegment"), tr("Text Segment"));
    m_textSegment->setCheckable(true);
    m_textSegment->setActionGroup(m_formatStyleGroup);

    m_textIndent = this->addAction(icons->icon("textIndent"), tr("First Line Indent"));
    m_textIndent->setCheckable(true);
    m_textIndent->setActionGroup(m_formatStyleGroup);

    m_blockIndent = this->addAction(icons->icon("blockIndent"), tr("Indent Paragraph"));
    m_blockOutdent = this->addAction(icons->icon("blockOutdent"), tr("Outdent Paragraph"));

    connect(m_textQuote, &QAction::triggered, [this]{emitDocumentAction(DocAction::BlockQuote);});
    connect(m_textSegment, &QAction::triggered, [this]{emitDocumentAction(DocAction::TextSegment);});
    connect(m_textIndent, &QAction::triggered, [this]{emitDocumentAction(DocAction::TextIndent);});
    connect(m_blockIndent, &QAction::triggered, [this]{emitDocumentAction(DocAction::BlockIndent);});
    connect(m_blockOutdent, &QAction::triggered, [this]{emitDocumentAction(DocAction::BlockOutdent);});
}

/**
 * Private Slots
 * =============
 */

void GuiEditToolBar::emitDocumentAction(DocAction action) {
    emit documentAction(action);
}

} // namespace Collett
