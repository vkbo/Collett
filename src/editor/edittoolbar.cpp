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

#include <QSize>
#include <QObject>
#include <QWidget>
#include <QToolBar>
#include <QSizePolicy>

namespace Collett {

GuiEditToolBar::GuiEditToolBar(QWidget *parent)
    : QToolBar(parent)
{
    CollettIcons *icons = CollettIcons::instance();

    this->setIconSize(QSize(20, 20));

    m_formatBold = this->addAction(icons->icon("bold"), tr("Bold"));
    m_formatItalic = this->addAction(icons->icon("italic"), tr("Italic"));
    m_formatUnderline = this->addAction(icons->icon("underline"), tr("Underline"));
    m_formatStrikethrough = this->addAction(icons->icon("strikethrough"), tr("Strikethrough"));

    connect(m_formatBold, &QAction::triggered, [this]{emitDocumentAction(DocAction::FormatBold);});
    connect(m_formatItalic, &QAction::triggered, [this]{emitDocumentAction(DocAction::FormatItalic);});
    connect(m_formatUnderline, &QAction::triggered, [this]{emitDocumentAction(DocAction::FormatUnderline);});
    connect(m_formatStrikethrough, &QAction::triggered, [this]{emitDocumentAction(DocAction::FormatStrikethrough);});

    this->addSeparator();

    m_alignLeft = this->addAction(icons->icon("alignLeft"), tr("Align Left"));
    m_alignCentre = this->addAction(icons->icon("alignCentre"), tr("Align Right"));
    m_alignRight = this->addAction(icons->icon("alignRight"), tr("Align Centre"));
    m_alignJustify = this->addAction(icons->icon("alignJustify"), tr("Align Justify"));

    connect(m_alignLeft, &QAction::triggered, [this]{emitDocumentAction(DocAction::TextAlignLeft);});
    connect(m_alignCentre, &QAction::triggered, [this]{emitDocumentAction(DocAction::TextAlignCentre);});
    connect(m_alignRight, &QAction::triggered, [this]{emitDocumentAction(DocAction::TextAlignRight);});
    connect(m_alignJustify, &QAction::triggered, [this]{emitDocumentAction(DocAction::TextAlignJustify);});

    this->addSeparator();

    m_blockIndent = this->addAction(icons->icon("blockIndent"), tr("Indent Paragraph"));
    m_blockOutdent = this->addAction(icons->icon("blockOutdent"), tr("Outdent Paragraph"));

    connect(m_blockIndent, &QAction::triggered, [this]{emitDocumentAction(DocAction::TextIndent);});
    connect(m_blockOutdent, &QAction::triggered, [this]{emitDocumentAction(DocAction::TextOutdent);});
}

void GuiEditToolBar::emitDocumentAction(DocAction action) {
    emit documentAction(action);
}

} // namespace Collett
