/*
** Collett – GUI Editor Tool Bar Class
** ===================================
**
** This file is a part of Collett
** Copyright 2021–2022, Veronica Berglyd Olsen
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

    m_formatParagraph = m_formatTextMenu->addAction(tr("Paragraph"));
    m_formatParagraph->setCheckable(true);
    m_formatParagraph->setActionGroup(m_formatTextGroup);

    m_formatBlockQuote = m_formatTextMenu->addAction(tr("Block Quote"));
    m_formatBlockQuote->setCheckable(true);
    m_formatBlockQuote->setActionGroup(m_formatTextGroup);

    m_formatText = new QToolButton(this);
    m_formatText->setIcon(icons->icon("formatText"));
    m_formatText->setText(tr("Format"));
    m_formatText->setMenu(m_formatTextMenu);
    m_formatText->setPopupMode(QToolButton::InstantPopup);
    this->addWidget(m_formatText);

    this->addSeparator();

    // Character Format Buttons
    // ========================

    m_formatBold = this->addAction(icons->icon("formatBold"), tr("Bold"));
    m_formatBold->setCheckable(true);

    m_formatItalic = this->addAction(icons->icon("formatItalic"), tr("Italic"));
    m_formatItalic->setCheckable(true);

    m_formatUnderline = this->addAction(icons->icon("formatUnderline"), tr("Underline"));
    m_formatUnderline->setCheckable(true);

    m_formatStrikeOut = this->addAction(icons->icon("formatStrikeOut"), tr("Strike Out"));
    m_formatStrikeOut->setCheckable(true);

    this->addSeparator();

    m_formatSuperScript = this->addAction(icons->icon("formatSupScript"), tr("Superscript"));
    m_formatSuperScript->setCheckable(true);

    m_formatSubScript = this->addAction(icons->icon("formatSubScript"), tr("Subscript"));
    m_formatSubScript->setCheckable(true);

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

    this->addSeparator();

    // Text Block Indentation and Style
    // ================================

    m_textSegment = this->addAction(icons->icon("textSegment"), tr("Text Segment"));
    m_textSegment->setCheckable(true);

    m_textIndent = this->addAction(icons->icon("textIndent"), tr("First Line Indent"));
    m_textIndent->setCheckable(true);

    m_blockIndent = this->addAction(icons->icon("blockIndent"), tr("Indent Paragraph"));
    m_blockOutdent = this->addAction(icons->icon("blockOutdent"), tr("Outdent Paragraph"));
}

} // namespace Collett
