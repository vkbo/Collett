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
#include "settings.h"
#include "doceditor.h"
#include "textedit.h"
#include "document.h"
#include "edittoolbar.h"

#include <QFont>
#include <QTime>
#include <QUuid>
#include <QTimer>
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
    m_document = nullptr;

    m_textArea = new GuiTextEdit(this);
    m_textArea->setReadOnly(true);

    m_editToolBar = new GuiEditToolBar(this);

    QVBoxLayout *outerBox = new QVBoxLayout;
    outerBox->addWidget(m_editToolBar);
    outerBox->addWidget(m_textArea);
    outerBox->setContentsMargins(0, 0, 0, 0);
    outerBox->setSpacing(0);

    this->setLayout(outerBox);

    CollettSettings *settings = CollettSettings::instance();

    // Timers
    m_autoSave = new QTimer(this);
    m_autoSave->setInterval(settings->editorAutoSave() * 1000);

    // Connections

    connect(m_editToolBar, SIGNAL(documentAction(DocAction)),
            m_textArea, SLOT(applyDocAction(DocAction)));
    connect(m_textArea, SIGNAL(currentCharFormatChanged(const QTextCharFormat&)),
            this, SLOT(editorCharFormatChanged(const QTextCharFormat&)));
    connect(m_textArea, SIGNAL(currentBlockChanged(const QTextBlock&)),
            this, SLOT(editorBlockChanged(const QTextBlock&)));
    connect(m_autoSave, SIGNAL(timeout()),
            this, SLOT(flushEditorData()));
}

/**
 * Data Methods
 * ============
 */

bool GuiDocEditor::openDocument(const QUuid &uuid) {

    if (!m_data->hasProject()) {
        qWarning() << "No project loaded";
        return false;
    }

    m_docUuid = uuid;
    m_document = m_data->project()->document(uuid);
    m_textArea->setJsonContent(m_document->content());

    m_autoSave->start();
    m_textArea->setReadOnly(false);

    return true;
}

bool GuiDocEditor::saveDocument() {

    if (!m_data->hasProject()) {
        qWarning() << "No project loaded";
        return false;
    }
    if (!hasDocument()) {
        qWarning() << "No document to save";
        return false;
    }

    m_document->setLocked(true);
    m_document->setContent(m_textArea->toJsonContent());
    m_document->setLocked(false);

    return true;
}

void GuiDocEditor::closeDocument() {
    m_autoSave->stop();
    m_textArea->setReadOnly(true);
    m_textArea->clear();
    m_docUuid = QUuid();
    m_document = nullptr;
}

/**
 * Status Methods
 * ==============
 */

QUuid GuiDocEditor::currentDocument() const {
    return m_docUuid;
}

bool GuiDocEditor::hasDocument() const {
    return m_document != nullptr && !m_docUuid.isNull();
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
    m_editToolBar->m_alignCentre->setChecked(blockFormat.alignment() == Qt::AlignHCenter);
    m_editToolBar->m_alignRight->setChecked(blockFormat.alignment() == Qt::AlignRight);
    m_editToolBar->m_alignJustify->setChecked(blockFormat.alignment() == Qt::AlignJustify);
    m_editToolBar->m_textIndent->setChecked(blockFormat.textIndent() > 0.0);
}

void GuiDocEditor::flushEditorData() {

    qDebug() << "Ding!";

    if (!m_data->hasProject() || !hasDocument()) {
        return;
    }

    if (m_textArea->isModified()) {
        qDebug() << "Autosaving editor content";
        m_document->setLocked(true);
        m_document->setContent(m_textArea->toJsonContent());
        m_document->setLocked(false);
        if (m_document->write()) {
            m_textArea->setModified(false);
        }
    }
}

} // namespace Collett
