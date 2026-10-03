/*
** Collett - Document Binder Class
** ===============================
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

#include "documentbinder.h"
#include "settings.h"
#include "textcheck.h"

#include <QAbstractTextDocumentLayout>
#include <QQuickTextDocument>
#include <QSizeF>
#include <QTextBlock>
#include <QTextCursor>

namespace Collett {

// Constructor/Destructor
// ======================

DocumentBinder::DocumentBinder(QObject *parent) : QObject(parent)
{
    m_highlighter = new Highlighter(this);
    m_highlighter->setErrorColor(m_spellErrorColor);
    m_highlighter->setFormatErrorColor(m_formatErrorColor);
    m_highlighter->setCheckFormat(Settings::instance()->showMultiSpaces());
    connect(Settings::instance(), &Settings::showMultiSpacesChanged, this, [this]() {
        m_highlighter->setCheckFormat(Settings::instance()->showMultiSpaces());
    });
    connect(Settings::instance(), &Settings::textFormatChanged, this, &DocumentBinder::textFontChanged);
}

DocumentBinder::~DocumentBinder() {}

// Getters
// =======

/**! @brief The font of the document text, for text shown alongside it.
 */
QFont DocumentBinder::textFont() const
{
    return Settings::instance()->textFont();
}

QFont DocumentBinder::headingFont() const
{
    return Settings::instance()->headingFont();
}

// Setters
// =======

/**! @brief Set the TextEdit item that displays the document.
 */
void DocumentBinder::setTarget(QQuickItem *target)
{
    if (m_target == target) return;
    if (m_target) m_target->disconnect(this);
    m_target = target;
    if (m_target) {
        connect(m_target, SIGNAL(cursorPositionChanged()), this, SLOT(updateFormat()));
        connect(m_target, SIGNAL(cursorPositionChanged()), this, SLOT(updateCursor()));
        connect(m_target, SIGNAL(selectionStartChanged()), this, SLOT(updateFormat()));
        connect(m_target, SIGNAL(selectionEndChanged()), this, SLOT(updateFormat()));
    }
    emit targetChanged();
    openDocument();
}

/**! @brief Set the project the document is opened from.
 */
void DocumentBinder::setProject(Project *project)
{
    if (m_project == project) return;
    if (m_project) m_project->disconnect(this);
    m_project = project;
    if (m_project) connect(m_project, &Project::documentDeleting, this, &DocumentBinder::releaseDocument);
    m_highlighter->setSpellChecker(m_project ? m_project->spellChecker() : nullptr);
    emit projectChanged();
    openDocument();
}

/**! @brief Set the handle of the document to show.
 */
void DocumentBinder::setHandle(const QString &handle)
{
    if (m_handle == handle) return;
    m_handle = handle;
    emit handleChanged();
    openDocument();
}

/**! @brief Set the colour of the line under misspelled words.
 */
void DocumentBinder::setSpellErrorColor(const QColor &color)
{
    if (m_spellErrorColor == color) return;
    m_spellErrorColor = color;
    m_highlighter->setErrorColor(color);
    emit spellErrorColorChanged();
}

/**! @brief Set the colour of the line under redundant spaces.
 */
void DocumentBinder::setFormatErrorColor(const QColor &color)
{
    if (m_formatErrorColor == color) return;
    m_formatErrorColor = color;
    m_highlighter->setFormatErrorColor(color);
    emit formatErrorColorChanged();
}

/**! @brief Make the selection or the word at the cursor bold, or not.
 */
void DocumentBinder::setBold(bool bold)
{
    QTextCharFormat format;
    format.setFontWeight(bold ? QFont::Bold : QFont::Normal);
    mergeFormat(format);
}

void DocumentBinder::setItalic(bool italic)
{
    QTextCharFormat format;
    format.setFontItalic(italic);
    mergeFormat(format);
}

void DocumentBinder::setUnderline(bool underline)
{
    QTextCharFormat format;
    format.setFontUnderline(underline);
    mergeFormat(format);
}

void DocumentBinder::setStrikeOut(bool strikeOut)
{
    QTextCharFormat format;
    format.setFontStrikeOut(strikeOut);
    mergeFormat(format);
}

void DocumentBinder::setSuperscript(bool superscript)
{
    QTextCharFormat format;
    format.setVerticalAlignment(superscript ? QTextCharFormat::AlignSuperScript : QTextCharFormat::AlignNormal);
    mergeFormat(format);
}

void DocumentBinder::setSubscript(bool subscript)
{
    QTextCharFormat format;
    format.setVerticalAlignment(subscript ? QTextCharFormat::AlignSubScript : QTextCharFormat::AlignNormal);
    mergeFormat(format);
}

/**! @brief Set the alignment of the paragraphs in the selection, or of the
 * paragraph at the cursor.
 */
void DocumentBinder::setAlignment(int alignment)
{
    QTextCursor cursor = targetCursor();
    if (cursor.isNull()) return;

    QTextBlockFormat format;
    format.setAlignment(Qt::Alignment(alignment));
    cursor.mergeBlockFormat(format);
    updateFormat();
}

/**! @brief Make the paragraphs in the selection, or the paragraph at the
 * cursor, plain paragraphs (level 0) or headings (level 1 to 4).
 */
void DocumentBinder::setHeadingLevel(int level)
{
    QTextCursor cursor = targetCursor();
    if (cursor.isNull()) return;

    m_document->setHeadingLevel(cursor.selectionStart(), cursor.selectionEnd(), qBound(0, level, 4));
    updateFormat();
}

// Formatting
// ==========

/**! @brief Increase the block indent of the paragraphs in the selection.
 */
void DocumentBinder::indent()
{
    changeIndent(1);
}

/**! @brief Decrease the block indent of the paragraphs in the selection.
 */
void DocumentBinder::outdent()
{
    changeIndent(-1);
}

/**! @brief Start a new paragraph when Enter is pressed, if the new one
 * should not just copy the format of the current one.
 *
 * Enter at the end of a heading starts a plain paragraph. With automatic
 * indent on, a new paragraph after a text paragraph gets a first-line
 * indent, unless the text is centred or right-aligned. The first paragraph
 * after a heading is not indented. Returns false, and does nothing, in all
 * other cases, so the key can be handled as usual.
 */
bool DocumentBinder::newParagraph()
{
    QTextCursor cursor = targetCursor();
    if (cursor.isNull() || cursor.hasSelection()) return false;

    const Settings::TextFormat format = Settings::instance()->textFormat();
    const QTextBlockFormat current = cursor.blockFormat();
    if (current.headingLevel() > 0) {
        if (!cursor.atBlockEnd()) return false;
        cursor.insertBlock(format.blockParagraph, format.charParagraph);
    } else if (Settings::instance()->textAutoIndent()) {
        const Qt::Alignment align = current.alignment() & Qt::AlignHorizontal_Mask;
        if (align == Qt::AlignHCenter || align == Qt::AlignCenter || align == Qt::AlignRight) return false;
        QTextBlockFormat indented = current;
        indented.setTextIndent(format.tabWidth);
        cursor.insertBlock(indented);
    } else {
        return false;
    }
    m_target->setProperty("cursorPosition", cursor.position());
    return true;
}

/**! @brief Give the paragraph at the cursor a first-line indent, if the
 * cursor is at its start and it has none.
 *
 * Headings are never indented. Returns false, and does nothing, if the
 * indent was not added.
 */
bool DocumentBinder::addFirstLineIndent()
{
    QTextCursor cursor = targetCursor();
    if (cursor.isNull() || cursor.hasSelection() || !cursor.atBlockStart()) return false;

    QTextBlockFormat format = cursor.blockFormat();
    if (format.headingLevel() > 0 || format.textIndent() > 0.0) return false;
    format.setTextIndent(Settings::instance()->textFormat().tabWidth);
    cursor.setBlockFormat(format);
    return true;
}

/**! @brief Remove the first-line indent of the paragraph at the cursor, if
 * the cursor is at its start and it has one.
 *
 * Returns false, and does nothing, if there was no indent to remove.
 */
bool DocumentBinder::removeFirstLineIndent()
{
    QTextCursor cursor = targetCursor();
    if (cursor.isNull() || cursor.hasSelection() || !cursor.atBlockStart()) return false;

    QTextBlockFormat format = cursor.blockFormat();
    if (format.textIndent() <= 0.0) return false;
    format.setTextIndent(0.0);
    cursor.setBlockFormat(format);
    return true;
}

// Spell Checking
// ==============

/**! @brief The misspelled word at a position in the document, if any.
 *
 * Returns the word, its start and end positions in the document, and the
 * spelling suggestions for it, or an empty map if the position is not on a
 * misspelled word.
 */
QVariantMap DocumentBinder::misspelledWordAt(int position) const
{
    if (!m_document || !m_project) return {};

    const QTextBlock block = m_document->findBlock(position);
    if (!block.isValid()) return {};

    SpellChecker *spell = m_project->spellChecker();
    const int offset = position - block.position();
    const TextCheckList errors = spellCheckText(block.text(), spell);
    for (const TextCheck &error : errors) {
        if (offset >= error.start && offset <= error.end) {
            return {
                {"word", error.text},
                {"start", block.position() + error.start},
                {"end", block.position() + error.end},
                {"suggestions", spell->suggestWords(error.text)},
            };
        }
    }
    return {};
}

/**! @brief Replace a range of the document text, as one undo step.
 *
 * The new text takes the format of the text it replaces.
 */
void DocumentBinder::replaceText(int start, int end, const QString &text)
{
    if (!m_document) return;
    QTextCursor cursor(m_document);
    cursor.setPosition(start);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
    cursor.insertText(text);
}

/**! @brief Add a word to the project's user dictionary.
 */
bool DocumentBinder::addWord(const QString &word)
{
    return m_project && m_project->spellChecker()->addWord(word);
}

// Internal Functions
// ==================

/**! @brief Open the document from the project and bind it.
 *
 * Nothing happens until the target, the project and the handle are all set.
 * The project owns the document, and keeps it open after it is replaced.
 */
void DocumentBinder::openDocument()
{
    if (!m_target || !m_project || m_handle.isEmpty()) return;

    Document *document = m_project->openDocument(m_handle);
    if (document) {
        bindDocument(document);
    }
}

/**! @brief Make the document the text document of the target TextEdit.
 *
 * When the TextEdit is given a document from outside, it does not connect
 * the signals it connects for the document it creates itself. Without them,
 * format-only changes, like those from a syntax highlighter, are not
 * repainted, and the undo and redo state is not reported. They are connected
 * here by name, as the slots are private. The page size and margin set on
 * the TextEdit's own document are also copied over. The connections to the
 * previous document are removed, as it stays open in the project.
 */
void DocumentBinder::bindDocument(Document *document)
{
    if (document == m_document) return;

    QQuickTextDocument *textDocument = m_target->property("textDocument").value<QQuickTextDocument *>();
    if (!textDocument) {
        qWarning() << "Document binder target is not a TextEdit";
        return;
    }

    if (m_document) {
        m_document->disconnect(m_target);
        m_document->disconnect(this);
        m_document->documentLayout()->disconnect(m_target);
    }

    m_document = document;
    m_document->setPageSize(QSizeF(0, 0));

    // The margin is a format change on the document's root frame, so it would
    // otherwise mark the text as modified and add an undo step
    const qreal margin = m_target->property("textMargin").toReal();
    if (m_document->documentMargin() != margin) {
        const bool modified = m_document->isModified();
        m_document->setUndoRedoEnabled(false);
        m_document->setDocumentMargin(margin);
        m_document->setUndoRedoEnabled(true);
        m_document->setModified(modified);
    }

    textDocument->setTextDocument(m_document);
    m_highlighter->setDocument(m_document);

    connect(m_document, SIGNAL(contentsChange(int, int, int)), m_target, SLOT(q_contentsChange(int, int, int)));
    connect(m_document->documentLayout(), SIGNAL(updateBlock(QTextBlock)), m_target, SLOT(invalidateBlock(QTextBlock)));
    connect(m_document, SIGNAL(undoAvailable(bool)), m_target, SIGNAL(canUndoChanged()));
    connect(m_document, SIGNAL(redoAvailable(bool)), m_target, SIGNAL(canRedoChanged()));

    connect(m_document, &QTextDocument::contentsChange, this, &DocumentBinder::applyPending);
    m_pendingPosition = -1;
    updateFormat();
    updateCursor();
}

/**! @brief Swap the target over to an empty document when the shown
 * document is about to be deleted.
 *
 * The TextEdit cannot be without a document, and it may outlive the one it
 * shows, as delegates are destroyed or reused later.
 */
void DocumentBinder::releaseDocument(Document *document)
{
    if (!m_document || document != m_document) return;

    m_document->disconnect(m_target);
    m_document->disconnect(this);
    m_document->documentLayout()->disconnect(m_target);
    m_highlighter->setDocument(nullptr);
    m_document = nullptr;
    m_pendingPosition = -1;

    if (!m_placeholder) m_placeholder = new QTextDocument(this);
    if (m_target) {
        if (auto *textDocument = m_target->property("textDocument").value<QQuickTextDocument *>()) {
            textDocument->setTextDocument(m_placeholder);
        }
    }
}

/**! @brief A cursor in the document with the target's cursor and selection.
 *
 * The cursor is null if no document is shown.
 */
QTextCursor DocumentBinder::targetCursor() const
{
    if (!m_target || !m_document) return QTextCursor();

    const int last = m_document->characterCount() - 1;
    const int position = qBound(0, m_target->property("cursorPosition").toInt(), last);
    const int start = qBound(0, m_target->property("selectionStart").toInt(), last);
    const int end = qBound(0, m_target->property("selectionEnd").toInt(), last);

    QTextCursor cursor(m_document);
    if (start != end) {
        cursor.setPosition(position == start ? end : start);
        cursor.setPosition(position, QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(position);
    }
    return cursor;
}

/**! @brief Merge a character format into the selection, or into the word at
 * the cursor.
 *
 * When the cursor is not on a word, the format is held back and applied to
 * the text typed next at the cursor.
 */
void DocumentBinder::mergeFormat(const QTextCharFormat &format)
{
    QTextCursor cursor = targetCursor();
    if (cursor.isNull()) return;

    if (!cursor.hasSelection()) cursor.select(QTextCursor::WordUnderCursor);
    if (cursor.hasSelection()) {
        cursor.mergeCharFormat(format);
        m_pending = QTextCharFormat();
        m_pendingPosition = -1;
    } else {
        m_pending.merge(format);
        m_pendingPosition = cursor.position();
    }
    updateFormat();
}

/**! @brief Change the block indent of the paragraphs in the selection.
 *
 * The document format supports a block indent from 0 to 9.
 */
void DocumentBinder::changeIndent(int step)
{
    QTextCursor cursor = targetCursor();
    if (cursor.isNull()) return;

    QTextBlock block = m_document->findBlock(cursor.selectionStart());
    const QTextBlock last = m_document->findBlock(cursor.selectionEnd());
    cursor.beginEditBlock();
    while (block.isValid()) {
        QTextCursor edit(block);
        QTextBlockFormat format = block.blockFormat();
        format.setIndent(qBound(0, format.indent() + step, 9));
        edit.setBlockFormat(format);
        if (block == last) break;
        block = block.next();
    }
    cursor.endEditBlock();
}

/**! @brief Update the format reported for the target's cursor.
 *
 * A held back format is dropped when the cursor moves away from it.
 */
void DocumentBinder::updateFormat()
{
    if (m_applying) return;

    const QTextCursor cursor = targetCursor();
    QTextCharFormat format;
    int alignment = Qt::AlignLeft;
    int headingLevel = 0;
    if (!cursor.isNull()) {
        headingLevel = cursor.blockFormat().headingLevel();
        format = cursor.charFormat();
        alignment = int(cursor.blockFormat().alignment() & Qt::AlignHorizontal_Mask);
        if (alignment == Qt::AlignCenter) alignment = Qt::AlignHCenter;
    }
    if (m_pendingPosition >= 0 && (cursor.isNull() || cursor.hasSelection() || cursor.position() != m_pendingPosition)) {
        m_pending = QTextCharFormat();
        m_pendingPosition = -1;
    }
    format.merge(m_pending);

    if (format == m_format && alignment == m_alignment && headingLevel == m_headingLevel) return;
    m_format = format;
    m_alignment = alignment;
    m_headingLevel = headingLevel;
    emit formatChanged();
}

/**! @brief Tell the highlighter where the target's cursor is.
 */
void DocumentBinder::updateCursor()
{
    if (m_target && m_document) m_highlighter->setCursorPosition(m_target->property("cursorPosition").toInt());
}

/**! @brief Apply a held back format to text typed at its position.
 *
 * The document may report a change to a larger range than the typed text,
 * so the typed text is taken to start at the held back position, with the
 * length the change grew by. The format is joined to the typing in the undo
 * history.
 */
void DocumentBinder::applyPending(int position, int removed, int added)
{
    if (m_applying || m_pendingPosition < 0) return;
    const int typed = added - removed;
    if (typed <= 0 || m_pendingPosition < position || m_pendingPosition > position + removed) return;

    m_applying = true;
    QTextCursor cursor(m_document);
    cursor.setPosition(m_pendingPosition);
    cursor.setPosition(m_pendingPosition + typed, QTextCursor::KeepAnchor);
    cursor.joinPreviousEditBlock();
    cursor.mergeCharFormat(m_pending);
    cursor.endEditBlock();
    m_applying = false;

    m_pending = QTextCharFormat();
    m_pendingPosition = -1;
    updateFormat();
}

} // namespace Collett
