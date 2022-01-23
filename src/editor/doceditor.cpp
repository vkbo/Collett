/*
** Collett – GUI Document Editor Class
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
#include "doceditor.h"
#include "textedit.h"
#include "edittoolbar.h"

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>

namespace Collett {

GuiDocEditor::GuiDocEditor(QWidget *parent)
    : QWidget(parent)
{
    m_data = CollettData::instance();
    m_docUuid = QUuid();

    m_textArea = new GuiTextEdit(this);
    m_editToolBar = new GuiEditToolBar(this);
    connect(m_editToolBar, SIGNAL(documentAction(DocAction)), m_textArea, SLOT(applyDocAction(DocAction)));

    QVBoxLayout *outerBox = new QVBoxLayout;
    outerBox->addWidget(m_editToolBar);
    outerBox->addWidget(m_textArea);
    outerBox->setContentsMargins(0, 0, 0, 0);
    outerBox->setSpacing(0);

    this->setLayout(outerBox);
}

/**
 * Methods
 * =======
 */

bool GuiDocEditor::openDocument(const QUuid &uuid) {
    if (!m_data->hasProject()) {
        qWarning() << "No project loaded";
        return false;
    }

    m_textArea->setHtml(
        "<h3>Some Document</h3>"
        "<p><b>Hello World!</b></p>"
        "<p>This is a text <i>paragraph</i> with some <u>simple</u> formatting.</p>"
        "<p>Here is a paragraph with no formatting whatsoever.</p>"
        "<p>&nbsp;</p>"
        "<p>Blank paragraph above here.</p>"
    );

    m_docUuid = uuid;

    return true;
}

bool GuiDocEditor::saveDocument() {
    if (!m_data->hasProject()) {
        qWarning() << "No project loaded";
        return false;
    }
    if (m_docUuid.isNull()) {
        qWarning() << "No document to save";
        return false;
    }
    return m_data->project()->store()->saveFile(m_docUuid, m_textArea->toJsonObject());
}

} // namespace Collett
