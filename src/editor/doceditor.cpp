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

#include <QFont>
#include <QTime>
#include <QUuid>
#include <QObject>
#include <QWidget>
#include <QJsonObject>
#include <QVBoxLayout>
#include <QTextCharFormat>

namespace Collett {

GuiDocEditor::GuiDocEditor(QWidget *parent)
    : QWidget(parent)
{
    m_data = CollettData::instance();
    m_docUuid = QUuid();

    m_textArea = new GuiTextEdit(this);
    m_editToolBar = new GuiEditToolBar(this);

    QVBoxLayout *outerBox = new QVBoxLayout;
    outerBox->addWidget(m_editToolBar);
    outerBox->addWidget(m_textArea);
    outerBox->setContentsMargins(0, 0, 0, 0);
    outerBox->setSpacing(0);

    this->setLayout(outerBox);

    // Connections

    connect(m_editToolBar, SIGNAL(documentAction(DocAction)),
            m_textArea, SLOT(applyDocAction(DocAction)));
    connect(m_textArea, SIGNAL(currentCharFormatChanged(const QTextCharFormat&)),
            this, SLOT(editorCharFormatChanged(const QTextCharFormat&)));
    connect(m_textArea, SIGNAL(currentBlockChanged(const QTextBlock&)),
            this, SLOT(editorBlockChanged(const QTextBlock&)));
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
        "<p>Here is a paragraph with no formatting whatsoever.<br>This is a second line in the same paragraph.</p>"
        "<p>&nbsp;</p>"
        "<p>Blank paragraph above here.</p>"
        "<p style='text-align: center'>* * *</p>"
        "<p>This text belongs to a second section of the text document.</p>"
    );

    QJsonObject json;
    bool status = m_data->project()->store()->loadFile(uuid, json);
    m_textArea->setJsonObject(json);

    m_docUuid = uuid;

    return status;
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
    QTime startTime = QTime::currentTime();
    bool status = m_data->project()->store()->saveFile(m_docUuid, m_textArea->toJsonObject());
    QTime endTime = QTime::currentTime();
    qDebug() << "Save file took (ms):" << startTime.msecsTo(endTime);

    return status;
}

/**
 * Private Slots
 * =============
 */

void GuiDocEditor::editorCharFormatChanged(const QTextCharFormat &fmt) {
    m_editToolBar->m_formatBold->setChecked(fmt.fontWeight() > QFont::Medium);
    m_editToolBar->m_formatItalic->setChecked(fmt.fontItalic());
    m_editToolBar->m_formatUnderline->setChecked(fmt.fontUnderline());
    m_editToolBar->m_formatStrikethrough->setChecked(fmt.fontStrikeOut());
}

void GuiDocEditor::editorBlockChanged(const QTextBlock &block) {
    QTextBlockFormat blockFormat = block.blockFormat();
    m_editToolBar->m_alignLeft->setChecked(blockFormat.alignment() == Qt::AlignLeft);
    m_editToolBar->m_alignCentre->setChecked(blockFormat.alignment() == Qt::AlignCenter);
    m_editToolBar->m_alignRight->setChecked(blockFormat.alignment() == Qt::AlignRight);
    m_editToolBar->m_alignJustify->setChecked(blockFormat.alignment() == Qt::AlignJustify);
    m_editToolBar->m_textIndent->setChecked(blockFormat.textIndent() > 0.0);
}

} // namespace Collett
