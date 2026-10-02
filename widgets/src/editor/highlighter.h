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

#pragma once

#include "collett.h"

#include <QString>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

namespace Collett {

/**! @brief Applies theme colours to the document as layout overlays.
 *
 * The document already holds the semantic formats, like heading levels and
 * bold or italic runs. This class only decides how they look, by setting
 * overlay formats that are never stored with the document. It also keeps a
 * snapshot of each block's text in the block's user data for the background
 * spell and format checks, which are run elsewhere.
 */
class GuiDocHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit GuiDocHighlighter(QObject *parent = nullptr);
    ~GuiDocHighlighter();

    // Methods
    void updateTheme();

protected:
    void highlightBlock(const QString &text) override;

private:
    QTextCharFormat m_fmtHeader;
    QTextCharFormat m_fmtEmphasis;
    QTextCharFormat m_fmtComment;
};

} // namespace Collett
