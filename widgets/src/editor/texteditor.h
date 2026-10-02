/*
** Collett - Text Editor Class
** ===========================
**
** This file is a part of Collett
** Copyright (C) 2025 Veronica Berglyd Olsen
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
#include "counting.h"
#include "document.h"
#include "highlighter.h"
#include "runnables.h"
#include "spellchecker.h"
#include "textblock.h"

#include <QContextMenuEvent>
#include <QHash>
#include <QKeyEvent>
#include <QList>
#include <QMenu>
#include <QPoint>
#include <QResizeEvent>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextEdit>
#include <QTimer>

namespace Collett {

class GuiTextEditor : public QTextEdit
{
    Q_OBJECT

public:
    explicit GuiTextEditor(QWidget *parent = nullptr);
    ~GuiTextEditor();

    // Methods
    void openDocument(Document *doc);
    void updateTheme();
    void beginCheckPass();
    QMenu *buildContextMenu(const QPoint &pos);

    // Setters
    void setSpellChecker(SpellChecker *spell);
    void setCheckSpelling(bool enabled);
    void setCheckFormatting(bool enabled);

    // Getters
    bool checkSpelling() const { return m_checkSpelling; };
    bool checkFormatting() const { return m_checkFormatting; };

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

public slots:
    void toggleBold(bool bold);
    void toggleItalic(bool italic);
    void toggleUnderline(bool underline);
    void toggleStrikeOut(bool strikeOut);
    void toggleSuperscript(bool superscript);
    void toggleSubscript(bool subscript);
    void alignLeft();
    void alignCenter();
    void alignRight();
    void alignJustify();
    void blockIndent();
    void blockOutdent();
    void runDocumentTasks();

signals:
    void documentCountsChanged(const QString &handle, const Collett::TextCounts &counts);

private slots:
    void onContentsChange(int pos, int removed, int added);
    void onCountsReady(const Collett::TextCounts &counts);
    void dispatchTextCheck();
    void onTextCheckResults(int jobId, const QList<Collett::TextCheckResult> &results);
    void updateCheckMarkers();
    void restartMarkerTimer();
    void updateTextFont();

private:
    // A block queued in a text check job, with the data revision it was
    // snapshotted at so a stale result can be recognised.
    struct CheckBlock
    {
        QTextBlock block;
        TextBlockData *data = nullptr;
        int revision = 0;
    };

    // The extra selections built for a block's error list, kept so they are
    // only rebuilt when that list changes.
    struct SelectionCache
    {
        TextCheckList errors;
        QList<QTextEdit::ExtraSelection> selections;
    };

    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    bool spellErrorAt(int pos, QTextBlock &block, TextCheck &error) const;
    void correctWord(QTextCursor cursor, const QString &word);
    void addWord(const QString &word, bool save);
    void visibleBlockRange(QTextBlock &first, int &last) const;
    QList<QTextEdit::ExtraSelection> buildSelections(
        const QTextBlock &block, const QTextCharFormat &format, const TextCheckList &errors
    ) const;
    void applyExtraSelections();

    GuiDocHighlighter *m_highlighter = nullptr;
    SpellChecker *m_spell = nullptr;
    QMetaObject::Connection m_docConnection;
    QString m_docHandle;

    // Document Tasks
    WordCounterDispatcher *m_wordCounter = nullptr;
    QTimer *m_timerDocTasks = nullptr;
    bool m_docTasksPending = false;
    QString m_countHandle;

    // Text Checks
    bool m_checkSpelling = true;
    bool m_checkFormatting = true;
    TextCheckDispatcher *m_dispatcher = nullptr;
    QTimer *m_timerTextCheck = nullptr;
    QTimer *m_timerMarkers = nullptr;
    QHash<int, QTextBlock> m_dirtyBlocks;
    int m_checkJobId = 0;
    bool m_checkJobActive = false;
    QList<CheckBlock> m_checkJob;
    int m_checkPassPos = -1;

    // Error Markers
    QTextCharFormat m_spellErrorFormat;
    QTextCharFormat m_formatErrorFormat;
    QHash<TextBlockData *, SelectionCache> m_selCacheSpell;
    QHash<TextBlockData *, SelectionCache> m_selCacheFormat;
    QList<QTextEdit::ExtraSelection> m_spellSelections;
    QList<QTextEdit::ExtraSelection> m_formatSelections;
};
} // namespace Collett
