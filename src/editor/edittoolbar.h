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

#ifndef GUI_EDITTOOLBAR_H
#define GUI_EDITTOOLBAR_H

#include "collett.h"

#include <QMenu>
#include <QAction>
#include <QObject>
#include <QToolBar>
#include <QToolButton>
#include <QActionGroup>

namespace Collett {


class GuiDocEditor;
class GuiEditToolBar : public QToolBar
{
    Q_OBJECT

public:
    GuiEditToolBar(QWidget *parent=nullptr);
    ~GuiEditToolBar() {};

private:
    QActionGroup *m_formatTextGroup;
    QActionGroup *m_alignTextGroup;

    QToolButton *m_formatText;
    QMenu       *m_formatTextMenu;
    QAction     *m_formatHeader1;
    QAction     *m_formatHeader2;
    QAction     *m_formatHeader3;
    QAction     *m_formatHeader4;
    QAction     *m_formatParagraph;
    QAction     *m_formatBlockQuote;

    QAction *m_formatBold;
    QAction *m_formatItalic;
    QAction *m_formatUnderline;
    QAction *m_formatStrikeOut;

    QAction *m_formatSuperScript;
    QAction *m_formatSubScript;

    QAction *m_alignLeft;
    QAction *m_alignCentre;
    QAction *m_alignRight;
    QAction *m_alignJustify;

    QAction *m_textSegment;
    QAction *m_textIndent;
    QAction *m_blockIndent;
    QAction *m_blockOutdent;

    friend class GuiDocEditor;

};
} // namespace Collett

#endif // GUI_EDITTOOLBAR_H
