/*
** Collett - GUI Text Editor Class
** ===============================
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

#include "collett.h"
#include "highlighter.h"
#include "runnables.h"
#include "settings.h"
#include "textblock.h"
#include "texteditor.h"
#include "theme.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QFont>
#include <QKeyEvent>
#include <QMenu>
#include <QRect>
#include <QResizeEvent>
#include <QScrollBar>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>

using namespace Qt::Literals::StringLiterals;

namespace Collett {

// Number of blocks sent to the background text check per job
static const int CHECK_PASS_CHUNK = 100;

// Milliseconds of quiet before edited blocks are sent for checking
static const int TEXT_CHECK_DELAY = 300;

// Milliseconds of quiet before the error markers are refreshed
static const int MARKER_DELAY = 150;

// Maximum number of spelling suggestions shown in the context menu
static const int MAX_SUGGESTIONS = 15;

// Milliseconds after an edit before the document tasks, like the word
// counter, are run. Further edits in that window do not push it back.
static const int DOC_TASK_DELAY = 5000;

// Constructor/Destructor
// ======================

GuiTextEditor::GuiTextEditor(QWidget *parent) : QTextEdit(parent)
{
    m_highlighter = new GuiDocHighlighter(this);

    // Document Tasks
    m_wordCounter = new WordCounterDispatcher(this);
    connect(m_wordCounter, &WordCounterDispatcher::countsReady, this, &GuiTextEditor::onCountsReady);

    m_timerDocTasks = new QTimer(this);
    m_timerDocTasks->setSingleShot(true);
    m_timerDocTasks->setInterval(DOC_TASK_DELAY);
    connect(m_timerDocTasks, &QTimer::timeout, this, &GuiTextEditor::runDocumentTasks);

    // Text Checks
    m_dispatcher = new TextCheckDispatcher(this);
    connect(m_dispatcher, &TextCheckDispatcher::resultsReady, this, &GuiTextEditor::onTextCheckResults);

    m_timerTextCheck = new QTimer(this);
    m_timerTextCheck->setSingleShot(true);
    m_timerTextCheck->setInterval(TEXT_CHECK_DELAY);
    connect(m_timerTextCheck, &QTimer::timeout, this, &GuiTextEditor::dispatchTextCheck);

    m_timerMarkers = new QTimer(this);
    m_timerMarkers->setSingleShot(true);
    m_timerMarkers->setInterval(MARKER_DELAY);
    connect(m_timerMarkers, &QTimer::timeout, this, &GuiTextEditor::updateCheckMarkers);

    // The markers only cover the visible blocks, so they must follow
    // scrolling, and the cursor, since the word under it is not marked
    connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, &GuiTextEditor::restartMarkerTimer);
    connect(this, &QTextEdit::cursorPositionChanged, this, &GuiTextEditor::restartMarkerTimer);

    this->updateTheme();
}

GuiTextEditor::~GuiTextEditor()
{
    qDebug() << "Destructor: GuiTextEditor";
}

// Public Methods
// ==============

/**! @brief Show a document in the editor, or none if null.
 *
 * The highlighter follows the document and is run right away, so every block
 * has its data before the text check pass starts. Any check job in flight
 * for the previous document is dropped, and the document tasks are run at
 * once so the new document's counts are up to date.
 */
void GuiTextEditor::openDocument(Document *doc)
{
    if (m_docConnection) {
        disconnect(m_docConnection);
    }
    m_timerDocTasks->stop();
    m_docTasksPending = false;
    m_docHandle = doc != nullptr ? doc->handle() : QString();

    this->setDocument(doc);
    m_highlighter->setDocument(doc);
    this->setEnabled(doc != nullptr);
    if (doc != nullptr) {
        m_docConnection = connect(doc, &QTextDocument::contentsChange, this, &GuiTextEditor::onContentsChange);
        m_highlighter->rehighlight();
        m_docTasksPending = true;
        this->runDocumentTasks();
    }
    this->beginCheckPass();
}

/**! @brief Spell and format check the visible blocks, then the whole document.
 *
 * The visible blocks are checked at once on the GUI thread so their markers
 * show immediately. The rest of the document is checked in chunks on the
 * worker thread. Any earlier job or queued blocks are dropped. Call this when
 * the document, the language or the user dictionary changes.
 */
void GuiTextEditor::beginCheckPass()
{
    m_timerTextCheck->stop();
    m_dirtyBlocks.clear();
    m_checkJobActive = false;
    m_checkJob.clear();
    m_checkPassPos = -1;
    m_selCacheSpell.clear();
    m_selCacheFormat.clear();

    QTextDocument *doc = m_highlighter->document();
    if (doc != nullptr && (m_checkSpelling || m_checkFormatting)) {
        QTextBlock block;
        int last = -1;
        this->visibleBlockRange(block, last);
        while (block.isValid() && block.blockNumber() <= last) {
            if (TextBlockData *data = dynamic_cast<TextBlockData *>(block.userData())) {
                if (m_checkSpelling) {
                    data->setSpellErrors(spellCheckText(data->text(), m_spell));
                }
                if (m_checkFormatting) {
                    data->setFormatErrors(formatCheckText(data->text()));
                }
            }
            block = block.next();
        }
        m_checkPassPos = 0;
        qDebug() << "Text check starting background pass over" << doc->blockCount() << "blocks";
        this->dispatchTextCheck();
    }
    this->updateCheckMarkers();
}

/**! @brief Build the context menu for a position in the viewport.
 *
 * The standard edit actions come first. If the position is on a misspelled
 * word, the spelling suggestions follow, with actions to ignore the word for
 * the session or add it to the project dictionary. The caller owns the menu.
 */
QMenu *GuiTextEditor::buildContextMenu(const QPoint &pos)
{
    QMenu *menu = this->createStandardContextMenu(pos);

    QTextBlock block;
    TextCheck error;
    if (m_checkSpelling && m_spell != nullptr && this->spellErrorAt(this->cursorForPosition(pos).position(), block, error)) {
        qDebug() << "Word" << error.text << "is misspelled";
        QTextCursor cursor(block);
        cursor.setPosition(block.position() + error.start);
        cursor.setPosition(block.position() + error.end, QTextCursor::KeepAnchor);

        menu->addSeparator();
        const QStringList suggestions = m_spell->suggestWords(error.text);
        if (suggestions.isEmpty()) {
            menu->addAction(tr("No Suggestions"))->setEnabled(false);
        } else {
            menu->addAction(tr("Spelling Suggestion(s)"))->setEnabled(false);
            for (const QString &option : suggestions.first(qMin(suggestions.size(), qsizetype(MAX_SUGGESTIONS)))) {
                QAction *action = menu->addAction(u"– %1"_s.arg(option));
                connect(action, &QAction::triggered, this, [this, cursor, option]() { this->correctWord(cursor, option); });
            }
        }

        menu->addSeparator();
        QString word = error.text;
        QAction *ignore = menu->addAction(tr("Ignore Word"));
        connect(ignore, &QAction::triggered, this, [this, word]() { this->addWord(word, false); });
        QAction *add = menu->addAction(tr("Add Word to Dictionary"));
        connect(add, &QAction::triggered, this, [this, word]() { this->addWord(word, true); });
    }

    return menu;
}

// Setters
// =======

/**! @brief Set the spell checker used by the text checks.
 *
 * The editor rechecks the document whenever the checker's language or user
 * dictionary changes.
 */
void GuiTextEditor::setSpellChecker(SpellChecker *spell)
{
    if (m_spell != nullptr) {
        disconnect(m_spell, nullptr, this, nullptr);
    }
    m_spell = spell;
    m_dispatcher->setSpellChecker(spell);
    if (m_spell != nullptr) {
        connect(m_spell, &SpellChecker::languageChanged, this, &GuiTextEditor::beginCheckPass);
        connect(m_spell, &SpellChecker::userDictionaryChanged, this, &GuiTextEditor::beginCheckPass);
    }
    this->beginCheckPass();
}

void GuiTextEditor::setCheckSpelling(bool enabled)
{
    if (m_checkSpelling != enabled) {
        m_checkSpelling = enabled;
        this->beginCheckPass();
    }
}

void GuiTextEditor::setCheckFormatting(bool enabled)
{
    if (m_checkFormatting != enabled) {
        m_checkFormatting = enabled;
        this->beginCheckPass();
    }
}

/**! @brief Rebuild the theme-dependent formats and rehighlight.
 *
 * The spell and format error formats are used for the extra selections that
 * mark errors found by the background text checks.
 */
void GuiTextEditor::updateTheme()
{
    Theme *theme = Theme::instance();

    m_spellErrorFormat = QTextCharFormat();
    m_spellErrorFormat.setUnderlineColor(theme->getSyntaxColor(SyntaxSpellLine));
    m_spellErrorFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);

    m_formatErrorFormat = QTextCharFormat();
    m_formatErrorFormat.setUnderlineColor(theme->getSyntaxColor(SyntaxErrorLine));
    m_formatErrorFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);

    m_highlighter->updateTheme();
    if (m_highlighter->document() != nullptr) {
        m_highlighter->rehighlight();
    }

    // Cached selections carry the old formats
    m_selCacheSpell.clear();
    m_selCacheFormat.clear();
    this->updateCheckMarkers();
}

// Protected Methods
// =================

/**!
 * @brief Intercept key presses for first-line indent handling.
 *
 * Pressing Tab at the start of a paragraph block toggles on a first-line
 * indent instead of inserting a tab character. Pressing Backspace at the start
 * of a block that has a first-line indent removes it instead of merging with
 * the previous block. In all other cases the key is passed on to the base
 * class, so Tab elsewhere still inserts a literal tab.
 *
 * @param event The key event.
 */
void GuiTextEditor::keyPressEvent(QKeyEvent *event)
{
    QTextCursor cursor = this->textCursor();
    int key = event->key();

    if (key == Qt::Key_Tab && !cursor.hasSelection() && cursor.atBlockStart()) {
        QTextBlockFormat format = cursor.blockFormat();
        if (format.headingLevel() == 0 && format.textIndent() <= 0.0) {
            format.setTextIndent(Settings::instance()->textFormat().tabWidth);
            cursor.setBlockFormat(format);
            return;
        }
    } else if (key == Qt::Key_Backspace && !cursor.hasSelection() && cursor.atBlockStart()) {
        QTextBlockFormat format = cursor.blockFormat();
        if (format.textIndent() > 0.0) {
            format.setTextIndent(0.0);
            cursor.setBlockFormat(format);
            return;
        }
    }

    QTextEdit::keyPressEvent(event);
}

/**! @brief Refresh the error markers when the visible range changes size.
 */
void GuiTextEditor::resizeEvent(QResizeEvent *event)
{
    QTextEdit::resizeEvent(event);
    this->restartMarkerTimer();
}

void GuiTextEditor::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = this->buildContextMenu(event->pos());
    menu->exec(event->globalPos());
    delete menu;
}

// Public Slots
// ============

void GuiTextEditor::toggleBold(bool bold)
{
    QTextCharFormat format;
    format.setFontWeight(bold ? QFont::Bold : QFont::Normal);
    this->mergeFormatOnWordOrSelection(format);
}

void GuiTextEditor::toggleItalic(bool italic)
{
    QTextCharFormat format;
    format.setFontItalic(italic);
    this->mergeFormatOnWordOrSelection(format);
}

void GuiTextEditor::toggleUnderline(bool underline)
{
    QTextCharFormat format;
    format.setFontUnderline(underline);
    this->mergeFormatOnWordOrSelection(format);
}

void GuiTextEditor::toggleStrikeOut(bool strikeOut)
{
    QTextCharFormat format;
    format.setFontStrikeOut(strikeOut);
    this->mergeFormatOnWordOrSelection(format);
}

void GuiTextEditor::toggleSuperscript(bool superscript)
{
    QTextCharFormat format;
    format.setVerticalAlignment(superscript ? QTextCharFormat::AlignSuperScript : QTextCharFormat::AlignNormal);
    this->mergeFormatOnWordOrSelection(format);
}

void GuiTextEditor::toggleSubscript(bool subscript)
{
    QTextCharFormat format;
    format.setVerticalAlignment(subscript ? QTextCharFormat::AlignSubScript : QTextCharFormat::AlignNormal);
    this->mergeFormatOnWordOrSelection(format);
}

void GuiTextEditor::alignLeft()
{
    this->setAlignment(Qt::AlignLeft);
}

void GuiTextEditor::alignCenter()
{
    this->setAlignment(Qt::AlignHCenter);
}

void GuiTextEditor::alignRight()
{
    this->setAlignment(Qt::AlignRight);
}

void GuiTextEditor::alignJustify()
{
    this->setAlignment(Qt::AlignJustify);
}

/**!
 * @brief Increase the block indent of the current paragraph.
 *
 * The document format supports a block indent from 0 to 9, so the value is
 * clamped to a maximum of 9.
 */
void GuiTextEditor::blockIndent()
{
    QTextCursor cursor = this->textCursor();
    QTextBlockFormat format = cursor.blockFormat();
    format.setIndent(qMin(format.indent() + 1, 9));
    cursor.setBlockFormat(format);
}

/**!
 * @brief Decrease the block indent of the current paragraph.
 *
 * The document format supports a block indent from 0 to 9, so the value is
 * clamped to a minimum of 0.
 */
void GuiTextEditor::blockOutdent()
{
    QTextCursor cursor = this->textCursor();
    QTextBlockFormat format = cursor.blockFormat();
    format.setIndent(qMax(format.indent() - 1, 0));
    cursor.setBlockFormat(format);
}

/**! @brief Run the tasks that follow an edit, if any edit is pending.
 *
 * Currently this sends a snapshot of the document to the background word
 * counter. If a count is still in flight, the tasks are tried again after
 * the usual delay, so the edits made during the count are not lost.
 */
void GuiTextEditor::runDocumentTasks()
{
    QTextDocument *doc = m_highlighter->document();
    if (!m_docTasksPending || doc == nullptr || m_docHandle.isEmpty()) {
        return;
    }
    if (m_wordCounter->isBusy()) {
        m_timerDocTasks->start();
        return;
    }
    qDebug() << "Running document tasks";
    m_docTasksPending = false;
    m_countHandle = m_docHandle;
    m_wordCounter->count(TextCounter::snapshot(doc));
}

// Private Slots
// =============

/**! @brief Queue the blocks touched by an edit for the background text check.
 *
 * The highlighter has already re-snapshotted these blocks and cleared their
 * cached errors by the time this runs, so the markers are refreshed too. The
 * document tasks are also scheduled, unless they already are.
 */
void GuiTextEditor::onContentsChange(int pos, int removed, int added)
{
    Q_UNUSED(removed);
    QTextDocument *doc = m_highlighter->document();
    if (doc == nullptr) {
        return;
    }

    m_docTasksPending = true;
    if (!m_timerDocTasks->isActive()) {
        m_timerDocTasks->start();
    }

    if (!m_checkSpelling && !m_checkFormatting) {
        return;
    }
    QTextBlock block = doc->findBlock(pos);
    while (block.isValid() && block.position() <= pos + added) {
        m_dirtyBlocks.insert(block.blockNumber(), block);
        block = block.next();
    }
    m_timerTextCheck->start();
    m_timerMarkers->start();
}

/**! @brief Forward the counts from the background word counter.
 *
 * The result is dropped if the editor has moved on to another document
 * since the count was dispatched.
 */
void GuiTextEditor::onCountsReady(const TextCounts &counts)
{
    if (!m_countHandle.isEmpty() && m_countHandle == m_docHandle) {
        emit documentCountsChanged(m_countHandle, counts);
    }
}

/**! @brief Send the next batch of blocks to the background text check.
 *
 * Edited blocks are sent first, then the blocks of the document pass, which
 * is tracked by character position since the document can change between
 * dispatches. Only one job runs at a time.
 */
void GuiTextEditor::dispatchTextCheck()
{
    if (m_checkJobActive) {
        return;
    }
    QTextDocument *doc = m_highlighter->document();
    if (doc == nullptr) {
        return;
    }

    QList<CheckBlock> job;
    QList<TextCheckPayload> payload;

    while (!m_dirtyBlocks.isEmpty() && job.size() < CHECK_PASS_CHUNK) {
        auto it = m_dirtyBlocks.begin();
        QTextBlock block = it.value();
        m_dirtyBlocks.erase(it);
        if (block.isValid()) {
            if (TextBlockData *data = dynamic_cast<TextBlockData *>(block.userData())) {
                payload.append({int(job.size()), data->text()});
                job.append({block, data, data->revision()});
            }
        }
    }

    if (m_checkPassPos >= 0) {
        QTextBlock block = doc->findBlock(m_checkPassPos);
        while (block.isValid() && job.size() < CHECK_PASS_CHUNK) {
            if (TextBlockData *data = dynamic_cast<TextBlockData *>(block.userData())) {
                payload.append({int(job.size()), data->text()});
                job.append({block, data, data->revision()});
            }
            block = block.next();
        }
        m_checkPassPos = block.isValid() ? block.position() : -1;
        if (m_checkPassPos < 0) {
            qDebug() << "Text check background pass complete";
        }
    }

    if (!job.isEmpty()) {
        m_checkJobId++;
        m_checkJob = job;
        m_checkJobActive = true;
        m_dispatcher->dispatch(m_checkJobId, payload, m_checkSpelling, m_checkFormatting);
    }
}

/**! @brief Store the results of a background text check.
 *
 * The whole job is ignored if it is not the one in flight, and a block's
 * result is ignored if the block was edited or removed since it was sent.
 */
void GuiTextEditor::onTextCheckResults(int jobId, const QList<TextCheckResult> &results)
{
    if (!m_checkJobActive || jobId != m_checkJobId) {
        return;
    }
    m_checkJobActive = false;
    for (const TextCheckResult &result : results) {
        if (result.index < 0 || result.index >= m_checkJob.size()) {
            continue;
        }
        const CheckBlock &item = m_checkJob.at(result.index);
        if (item.block.isValid() && item.block.userData() == item.data && item.data->revision() == item.revision) {
            item.data->setSpellErrors(result.spellErrors);
            item.data->setFormatErrors(result.formatErrors);
        }
    }
    m_checkJob.clear();
    m_timerMarkers->start();
    this->dispatchTextCheck();
}

/**! @brief Rebuild the spell and format error markers for the visible blocks.
 *
 * The extra selections for a block are cached against its block data and
 * only rebuilt when the block's own error list has changed. The word under
 * the cursor is not marked, and neither is trailing space at the cursor,
 * since that is a natural state while typing.
 */
void GuiTextEditor::updateCheckMarkers()
{
    m_spellSelections.clear();
    m_formatSelections.clear();

    QTextDocument *doc = m_highlighter->document();
    if (doc != nullptr && (m_checkSpelling || m_checkFormatting)) {
        int cursorPos = this->textCursor().position();
        QTextBlock block;
        int last = -1;
        this->visibleBlockRange(block, last);
        while (block.isValid() && block.blockNumber() <= last) {
            TextBlockData *data = dynamic_cast<TextBlockData *>(block.userData());
            if (data != nullptr) {
                int position = block.position();

                if (m_checkSpelling) {
                    const TextCheckList errors = data->spellErrors();
                    SelectionCache &cache = m_selCacheSpell[data];
                    if (cache.errors != errors || cache.selections.size() != errors.size()) {
                        cache.errors = errors;
                        cache.selections = this->buildSelections(block, m_spellErrorFormat, errors);
                    }
                    for (qsizetype i = 0; i < errors.size(); ++i) {
                        const TextCheck &error = errors.at(i);
                        if (position + error.start < cursorPos && cursorPos <= position + error.end) {
                            continue;
                        }
                        m_spellSelections.append(cache.selections.at(i));
                    }
                }

                if (m_checkFormatting) {
                    const TextCheckList errors = data->formatErrors();
                    SelectionCache &cache = m_selCacheFormat[data];
                    if (cache.errors != errors || cache.selections.size() != errors.size()) {
                        cache.errors = errors;
                        cache.selections = this->buildSelections(block, m_formatErrorFormat, errors);
                    }
                    for (qsizetype i = 0; i < errors.size(); ++i) {
                        const TextCheck &error = errors.at(i);
                        if (error.text == "trail"_L1 && position + error.start < cursorPos &&
                            cursorPos <= position + error.end) {
                            continue;
                        }
                        m_formatSelections.append(cache.selections.at(i));
                    }
                }
            }
            block = block.next();
        }
    }

    this->applyExtraSelections();
}

void GuiTextEditor::restartMarkerTimer()
{
    m_timerMarkers->start();
}

// Private Methods
// ===============

void GuiTextEditor::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = this->textCursor();
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }
    cursor.mergeCharFormat(format);
    this->mergeCurrentCharFormat(format);
}

/**! @brief Look up the misspelled word at a document position, if any.
 *
 * Uses the errors cached in the block data by the last check, so the word is
 * one that is currently marked. On success the block and the error, with
 * positions relative to the block, are returned.
 */
bool GuiTextEditor::spellErrorAt(int pos, QTextBlock &block, TextCheck &error) const
{
    QTextDocument *doc = m_highlighter->document();
    if (doc == nullptr) {
        return false;
    }
    block = doc->findBlock(pos);
    TextBlockData *data = dynamic_cast<TextBlockData *>(block.userData());
    if (!block.isValid() || data == nullptr) {
        return false;
    }
    int check = pos - block.position();
    const TextCheckList errors = data->spellErrors();
    for (const TextCheck &item : errors) {
        if (item.start <= check && check <= item.end) {
            error = item;
            return true;
        }
    }
    return false;
}

/**! @brief Replace the selected misspelled word with a suggestion.
 *
 * The cursor is left at the start of the replaced word.
 */
void GuiTextEditor::correctWord(QTextCursor cursor, const QString &word)
{
    int pos = cursor.selectionStart();
    cursor.beginEditBlock();
    cursor.removeSelectedText();
    cursor.insertText(word);
    cursor.endEditBlock();
    cursor.setPosition(pos);
    this->setTextCursor(cursor);
}

/**! @brief Add a word to the project dictionary, or ignore it for the session.
 *
 * The spell checker signals the change, which triggers a recheck of the
 * document so the word's markers disappear.
 */
void GuiTextEditor::addWord(const QString &word, bool save)
{
    if (m_spell != nullptr) {
        qDebug() << "Adding" << word << "to project dictionary," << (save ? "saved" : "unsaved");
        m_spell->addWord(word, save);
    }
}

/**! @brief Find the first visible block and the number of the last one.
 */
void GuiTextEditor::visibleBlockRange(QTextBlock &first, int &last) const
{
    QRect rect = this->viewport()->rect();
    first = this->cursorForPosition(rect.topLeft()).block();
    last = this->cursorForPosition(rect.bottomLeft()).blockNumber();
}

/**! @brief Build extra selections from a block's error positions.
 *
 * The cursors track the document, so the selections stay in place as edits
 * elsewhere shift the block's absolute position.
 */
QList<QTextEdit::ExtraSelection> GuiTextEditor::buildSelections(
    const QTextBlock &block, const QTextCharFormat &format, const TextCheckList &errors
) const
{
    QList<QTextEdit::ExtraSelection> selections;
    selections.reserve(errors.size());
    int position = block.position();
    for (const TextCheck &error : errors) {
        QTextCursor cursor(block);
        cursor.setPosition(position + error.start);
        cursor.setPosition(position + error.end, QTextCursor::KeepAnchor);
        QTextEdit::ExtraSelection selection;
        selection.cursor = cursor;
        selection.format = format;
        selections.append(selection);
    }
    return selections;
}

/**! @brief Push all extra selections to the widget.
 */
void GuiTextEditor::applyExtraSelections()
{
    this->setExtraSelections(m_spellSelections + m_formatSelections);
}

} // namespace Collett
