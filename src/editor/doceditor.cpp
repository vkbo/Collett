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

#include "item.h"
#include "collett.h"
#include "document.h"
#include "settings.h"
#include "textedit.h"
#include "doceditor.h"
#include "edittoolbar.h"

#include <QFont>
#include <QTime>
#include <QUuid>
#include <QTimer>
#include <QObject>
#include <QWidget>
#include <QJsonObject>
#include <QVBoxLayout>
#include <QApplication>
#include <QTextCharFormat>

namespace Collett {

GuiDocEditor::GuiDocEditor(QWidget *parent)
    : QWidget(parent)
{
    m_data = CollettData::instance();
    m_item = nullptr;
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
    connect(m_textArea, SIGNAL(currentCharFormatChanged(const QTextCharFormat&)),
            this, SLOT(editorCharFormatChanged(const QTextCharFormat&)));
    connect(m_textArea, SIGNAL(currentBlockChanged(const QTextBlock&)),
            this, SLOT(editorBlockChanged(const QTextBlock&)));
    connect(m_autoSave, SIGNAL(timeout()),
            this, SLOT(flushEditorData()));

    // Editor Tool Bar Signals
    connect(m_editToolBar->m_formatBold, SIGNAL(triggered()),
            m_textArea, SLOT(toggleBoldFormat()));
    connect(m_editToolBar->m_formatItalic, SIGNAL(triggered()),
            m_textArea, SLOT(toggleItalicFormat()));
    connect(m_editToolBar->m_formatUnderline, SIGNAL(triggered()),
            m_textArea, SLOT(toggleUnderlineFormat()));
    connect(m_editToolBar->m_formatStrikeOut, SIGNAL(triggered()),
            m_textArea, SLOT(toggleStrikeOutFormat()));
    connect(m_editToolBar->m_formatSuperScript, SIGNAL(triggered()),
            m_textArea, SLOT(toggleSuperScriptFormat()));
    connect(m_editToolBar->m_formatSubScript, SIGNAL(triggered()),
            m_textArea, SLOT(toggleSubScriptFormat()));

    connect(m_editToolBar->m_textSegment, SIGNAL(triggered()),
            m_textArea, SLOT(toggleSegmentFormat()));
    connect(m_editToolBar->m_textIndent, SIGNAL(triggered()),
            m_textArea, SLOT(toggleFirstLineIndent()));
    connect(m_editToolBar->m_blockIndent, SIGNAL(triggered()),
            m_textArea, SLOT(increaseBlockIndent()));
    connect(m_editToolBar->m_blockOutdent, SIGNAL(triggered()),
            m_textArea, SLOT(decreaseBlockIndent()));

    connect(m_editToolBar->m_formatHeader1, &QAction::triggered,
            [this]{m_textArea->applyBlockFormat(GuiTextEdit::Header, 1);});
    connect(m_editToolBar->m_formatHeader2, &QAction::triggered,
            [this]{m_textArea->applyBlockFormat(GuiTextEdit::Header, 2);});
    connect(m_editToolBar->m_formatHeader3, &QAction::triggered,
            [this]{m_textArea->applyBlockFormat(GuiTextEdit::Header, 3);});
    connect(m_editToolBar->m_formatHeader4, &QAction::triggered,
            [this]{m_textArea->applyBlockFormat(GuiTextEdit::Header, 4);});
    connect(m_editToolBar->m_formatParagraph, &QAction::triggered,
            [this]{m_textArea->applyBlockFormat(GuiTextEdit::Paragraph, 0);});
    connect(m_editToolBar->m_formatBlockQuote, &QAction::triggered,
            [this]{m_textArea->applyBlockFormat(GuiTextEdit::BlockQuote, 0);});

    connect(m_editToolBar->m_alignLeft, &QAction::triggered,
            [this]{m_textArea->applyBlockAlignment(Qt::AlignLeft);});
    connect(m_editToolBar->m_alignCentre, &QAction::triggered,
            [this]{m_textArea->applyBlockAlignment(Qt::AlignHCenter);});
    connect(m_editToolBar->m_alignRight, &QAction::triggered,
            [this]{m_textArea->applyBlockAlignment(Qt::AlignRight);});
    connect(m_editToolBar->m_alignJustify, &QAction::triggered,
            [this]{m_textArea->applyBlockAlignment(Qt::AlignJustify);});
}

/**
 * Data Methods
 * ============
 */

bool GuiDocEditor::openDocument(Item *item) {

    if (!m_data->hasProject() || !item) {
        qWarning() << "Nothing to load";
        return false;
    }

    m_item = item;
    m_document = m_data->project()->document(m_item->handle());
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
    if (!m_textArea->isModified()) {
        qDebug() << "Document has not been changed";
        return true;
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
    m_document = nullptr;
    m_item = nullptr;
}

/**
 * Status Methods
 * ==============
 */

QUuid GuiDocEditor::currentDocument() const {
    if (m_item) {
        return m_item->handle();
    } else {
        return QUuid();
    }
}

bool GuiDocEditor::hasDocument() const {
    return m_document != nullptr && m_item != nullptr;
}

/**!
 * @brief Check if this, or any child widget has focus
 */
bool GuiDocEditor::anyFocus() const {
    if (this->hasFocus())
        return true;
    return this->isAncestorOf(qApp->focusWidget());
}

/**
 * Private Slots
 * =============
 */

void GuiDocEditor::editorCharFormatChanged(const QTextCharFormat &fmt) {
    m_editToolBar->m_formatBold->setChecked(fmt.fontWeight() > QFont::Medium);
    m_editToolBar->m_formatItalic->setChecked(fmt.fontItalic());
    m_editToolBar->m_formatUnderline->setChecked(fmt.fontUnderline());
    m_editToolBar->m_formatStrikeOut->setChecked(fmt.fontStrikeOut());
    m_editToolBar->m_formatSuperScript->setChecked(fmt.verticalAlignment() == QTextCharFormat::AlignSuperScript);
    m_editToolBar->m_formatSubScript->setChecked(fmt.verticalAlignment() == QTextCharFormat::AlignSubScript);
}

void GuiDocEditor::editorBlockChanged(const QTextBlock &block) {

    QTextBlockFormat blockFormat = block.blockFormat();

    switch (blockFormat.headingLevel()) {
    case 1:
        m_editToolBar->m_formatHeader1->setChecked(true);
        break;
    case 2:
        m_editToolBar->m_formatHeader2->setChecked(true);
        break;
    case 3:
        m_editToolBar->m_formatHeader3->setChecked(true);
        break;
    case 4:
        m_editToolBar->m_formatHeader4->setChecked(true);
        break;
    default:
        m_editToolBar->m_formatParagraph->setChecked(true);
        break;
    }

    switch (blockFormat.alignment()) {
    case  Qt::AlignLeft:
        m_editToolBar->m_alignLeft->setChecked(true);
        break;
    case  Qt::AlignHCenter:
        m_editToolBar->m_alignCentre->setChecked(true);
        break;
    case  Qt::AlignRight:
        m_editToolBar->m_alignRight->setChecked(true);
        break;
    case  Qt::AlignJustify:
        m_editToolBar->m_alignJustify->setChecked(true);
        break;
    }

    m_editToolBar->m_textSegment->setChecked(blockFormat.textIndent() < 0.0);
    m_editToolBar->m_textIndent->setChecked(blockFormat.textIndent() > 0.0);
}

void GuiDocEditor::flushEditorData() {

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
