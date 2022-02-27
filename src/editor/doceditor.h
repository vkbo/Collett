/*
** Collett – GUI Document Editor Class
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

#ifndef GUI_DOCEDITOR_H
#define GUI_DOCEDITOR_H

#include "data.h"
#include "collett.h"
#include "document.h"
#include "textedit.h"
#include "edittoolbar.h"

#include <QTimer>
#include <QObject>
#include <QWidget>
#include <QTextBlock>
#include <QTextCharFormat>

namespace Collett {

class GuiDocEditor : public QWidget
{
    Q_OBJECT

public:
    GuiDocEditor(QWidget *parent=nullptr);
    ~GuiDocEditor() {};

    // Data Methods

    bool openDocument(const QString &path);
    bool saveDocument();
    void closeDocument();

    // Status Methods

    QUuid currentDocument() const;
    bool hasDocument() const;
    bool anyFocus() const;

signals:
    void popMessage(const Collett::Severity type, const QString &message);

private:
    GuiTextEdit    *m_textArea;
    GuiEditToolBar *m_editToolBar;
    QTimer         *m_autoSave;

    CollettData *m_data;
    Document    *m_document;

private slots:
    void editorCharFormatChanged(const QTextCharFormat &fmt);
    void editorBlockChanged(const QTextBlock &block);
    void flushEditorData();

};
} // namespace Collett

#endif // GUI_DOCEDITOR_H
