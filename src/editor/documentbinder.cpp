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
#include <QTextBlock>
#include <QTextCursor>
#include <QTextLayout>

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

/**! @brief The space above the first title, which is the divider from a
 * document before it. The editor scrolls it out of view.
 */
qreal DocumentBinder::topSpace() const
{
    if (!m_document) return 0.0;
    const Document::Item item = Document::itemOf(m_document->firstBlock());
    return Document::isTitle(m_document->firstBlock()) ? Document::dividerHeight(item.level, item.hardBreak) : 0.0;
}

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
        connect(m_target, SIGNAL(widthChanged()), this, SLOT(scheduleLayoutChanged()));
        connect(m_target, SIGNAL(contentSizeChanged()), this, SLOT(scheduleLayoutChanged()));
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
    if (m_project) {
        connect(m_project, &Project::documentDeleting, this, &DocumentBinder::releaseDocument);
        connect(m_project, &Project::projectChanged, this, &DocumentBinder::openDocument);
    }
    m_highlighter->setSpellChecker(m_project ? m_project->spellChecker() : nullptr);
    emit projectChanged();
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
 * paragraph at the cursor. Titles are left as they are.
 */
void DocumentBinder::setAlignment(int alignment)
{
    QTextCursor cursor = targetCursor();
    if (cursor.isNull()) return;

    QTextBlock block = m_document->findBlock(cursor.selectionStart());
    const QTextBlock last = m_document->findBlock(cursor.selectionEnd());
    cursor.beginEditBlock();
    while (block.isValid()) {
        if (!Document::isTitle(block)) {
            QTextCursor edit(block);
            QTextBlockFormat format;
            format.setAlignment(Qt::Alignment(alignment));
            edit.mergeBlockFormat(format);
        }
        if (block == last) break;
        block = block.next();
    }
    cursor.endEditBlock();
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

// Keys
// ====

/**! @brief Handle Enter, and return true if it was handled.
 *
 * Enter in a title moves the cursor to the start of the text, and adds a
 * paragraph if there is none. Ctrl+Enter in the text splits the document at
 * the cursor, and puts the cursor in the new title. Ctrl+Enter in that title
 * then cycles the new document's type. Other Enter presses in the text are
 * left to newParagraph, and a title never gets a line break.
 */
bool DocumentBinder::keyEnter(int modifiers)
{
    QTextCursor cursor = targetCursor();
    if (cursor.isNull() || !m_project) return false;

    const QTextBlock block = cursor.block();
    if (Document::isTitle(block)) {
        if (modifiers == Qt::ControlModifier) {
            upgradeSplit();
        } else if (modifiers == Qt::NoModifier) {
            const QTextBlock next = block.next();
            if (next.isValid() && !Document::isTitle(next)) {
                setCursor(next.position());
            } else {
                const Settings::TextFormat format = Settings::instance()->textFormat();
                cursor.clearSelection();
                cursor.setPosition(block.position() + block.length() - 1);
                cursor.insertBlock(format.blockParagraph, format.charParagraph);
                setCursor(cursor.position());
            }
        }
        return true;
    }

    if (modifiers == Qt::ControlModifier && !cursor.hasSelection()) {
        const QString handle = m_project->splitDocument(cursor.position());
        if (!handle.isEmpty()) {
            setCursor(titleBlock(handle).position());
            m_splitHandle = handle;
        }
        return true;
    }
    return modifiers == Qt::NoModifier && newParagraph();
}

/**! @brief Handle Backspace, and return true if it was handled.
 *
 * Backspace at the start of an indented paragraph removes the indent. At the
 * start of the text, it moves the cursor to the end of the title, and
 * removes the paragraph if it is the only one and empty. At the start of an
 * empty title, it merges the document into the one before it. A title is
 * never merged into the text before it, so it is not lost.
 */
bool DocumentBinder::keyBackspace()
{
    QTextCursor cursor = targetCursor();
    if (cursor.isNull() || cursor.hasSelection()) return false;

    const QTextBlock block = cursor.block();
    if (Document::isTitle(block)) {
        if (!cursor.atBlockStart()) return false;
        if (block.length() == 1) mergeUp(block);
        return true;
    }

    if (removeFirstLineIndent()) return true;
    const QTextBlock title = block.previous();
    if (!cursor.atBlockStart() || !Document::isTitle(title)) return false;

    if (title.length() == 1) {
        mergeUp(title);
    } else if (block.length() == 1 && !(block.next().isValid() && !Document::isTitle(block.next()))) {
        cursor.setPosition(title.position() + title.length() - 1);
        cursor.setPosition(block.position(), QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        setCursor(cursor.position());
    } else {
        setCursor(title.position() + title.length() - 1);
    }
    return true;
}

/**! @brief Handle Delete, and return true if it was handled.
 *
 * Delete at the end of the text of a document merges the next document into
 * it, if the next title is empty, and otherwise does nothing. Delete at the
 * end of a title does nothing, so text is not pulled into it.
 */
bool DocumentBinder::keyDelete()
{
    QTextCursor cursor = targetCursor();
    if (cursor.isNull() || cursor.hasSelection() || !cursor.atBlockEnd()) return false;

    const QTextBlock block = cursor.block();
    const QTextBlock next = block.next();
    if (Document::isTitle(block)) return next.isValid();
    if (!Document::isTitle(next)) return false;
    if (next.length() == 1 && m_project) {
        const int position = cursor.position();
        m_project->mergeDocument(Document::itemOf(next).handle);
        setCursor(position);
    }
    return true;
}

/**! @brief Handle Tab, and return true if it was handled.
 *
 * Tab at the start of a paragraph adds a first-line indent. A title takes
 * no tabs.
 */
bool DocumentBinder::keyTab()
{
    const QTextCursor cursor = targetCursor();
    if (cursor.isNull()) return false;
    return Document::isTitle(cursor.block()) || addFirstLineIndent();
}

// Documents
// =========

/**! @brief The rectangle of the first line of a document's title, in the
 * coordinates of the target, or an empty rectangle.
 */
QRectF DocumentBinder::titleRect(const QString &handle) const
{
    const QTextBlock block = titleBlock(handle);
    if (!block.isValid() || !block.layout() || block.layout()->lineCount() == 0) return QRectF();
    const QRectF rect = m_document->documentLayout()->blockBoundingRect(block);
    const QTextLine line = block.layout()->lineAt(0);
    return QRectF(rect.x(), rect.y() + line.y(), rect.width(), line.height());
}

/**! @brief The rectangle of a document's text, if it is a single empty
 * paragraph, or an empty rectangle.
 */
QRectF DocumentBinder::emptyBodyRect(const QString &handle) const
{
    const QTextBlock block = titleBlock(handle).next();
    if (!block.isValid() || Document::isTitle(block) || block.length() > 1) return QRectF();
    if (block.next().isValid() && !Document::isTitle(block.next())) return QRectF();
    if (!block.layout() || block.layout()->lineCount() == 0) return QRectF();
    const QRectF rect = m_document->documentLayout()->blockBoundingRect(block);
    const QTextLine line = block.layout()->lineAt(0);
    return QRectF(rect.x(), rect.y() + line.y(), rect.width(), line.height());
}

qreal DocumentBinder::dividerHeight(int level, bool hardBreak) const
{
    return Document::dividerHeight(ItemLevel(level), hardBreak);
}

qreal DocumentBinder::labelHeight() const
{
    return Document::labelHeight();
}

/**! @brief Put the cursor where a document opened from the project list
 * starts: the start of the text, or the end of the title if there is no
 * text but there is a title.
 */
void DocumentBinder::enterItem(const QString &handle)
{
    const QTextBlock title = titleBlock(handle);
    if (!title.isValid()) return;
    const QTextBlock next = title.next();
    const bool hasText = next.isValid() && !Document::isTitle(next) && !m_document->itemText(handle).isEmpty();
    if (!hasText && title.length() > 1) {
        setCursor(title.position() + title.length() - 1);
    } else if (next.isValid() && !Document::isTitle(next)) {
        setCursor(next.position());
    } else {
        setCursor(title.position());
    }
}

/**! @brief Put the cursor at the start of a document's title.
 */
void DocumentBinder::enterTitle(const QString &handle)
{
    const QTextBlock title = titleBlock(handle);
    if (title.isValid()) setCursor(title.position());
}

/**! @brief Put the cursor at a position in the text of a document, counted
 * from the start of its text.
 */
void DocumentBinder::enterText(const QString &handle, int position)
{
    const int start = itemPosition(handle);
    if (start >= 0) setCursor(qMin(start + position, m_document->itemEnd(handle)));
}

/**! @brief The position in the document where a document's text starts, or
 * the end of its title if it has no text, or -1.
 */
int DocumentBinder::itemPosition(const QString &handle) const
{
    const QTextBlock title = titleBlock(handle);
    if (!title.isValid()) return -1;
    const QTextBlock next = title.next();
    return next.isValid() && !Document::isTitle(next) ? next.position() : title.position() + title.length() - 1;
}

/**! @brief Delete a document. If the cursor was in it, it moves to the end
 * of the document before, or the start of the one after.
 */
bool DocumentBinder::deleteItem(const QString &handle)
{
    if (!m_document || !m_project) return false;
    const QTextBlock title = titleBlock(handle);
    const bool hadCursor = handle == m_currentHandle;
    const QString before = title.isValid() && title.position() > 0 ? m_document->handleAt(title.position() - 1) : QString();
    if (!m_project->deleteDocument(handle)) return false;
    if (!hadCursor) return true;
    if (!before.isEmpty()) {
        setCursor(m_document->itemEnd(before));
    } else {
        enterItem(Document::itemOf(m_document->firstBlock()).handle);
    }
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

/**! @brief Bind the text document of the group shown by the project.
 *
 * Nothing happens until the target and the project are set. The project
 * owns the document.
 */
void DocumentBinder::openDocument()
{
    if (!m_target || !m_project) return;

    Document *document = m_project->editorDocument();
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
 * here by name, as the slots are private. The width and margin set on the
 * TextEdit's own document are also copied over. The connections to the
 * previous document are removed.
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
        m_document->documentLayout()->disconnect(this);
    }

    // The document is laid out at the width of the TextEdit straight away,
    // as a long text is slow to lay out twice
    m_document = document;
    m_document->setTextWidth(m_target->width());

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

    // The TextEdit only draws the part of a large document that is in view
    // when it is told to, which keeps typing fast in a long text
    m_target->setFlag(QQuickItem::ItemObservesViewport, true);
    textDocument->setTextDocument(m_document);
    m_highlighter->setDocument(m_document);

    connect(m_document, SIGNAL(contentsChange(int, int, int)), m_target, SLOT(q_contentsChange(int, int, int)));
    connect(m_document->documentLayout(), SIGNAL(updateBlock(QTextBlock)), m_target, SLOT(invalidateBlock(QTextBlock)));
    connect(m_document, SIGNAL(undoAvailable(bool)), m_target, SIGNAL(canUndoChanged()));
    connect(m_document, SIGNAL(redoAvailable(bool)), m_target, SIGNAL(canRedoChanged()));

    connect(m_document, &QTextDocument::contentsChange, this, &DocumentBinder::applyPending);
    connect(m_document, &QTextDocument::contentsChanged, this, &DocumentBinder::updateTitles);
    connect(m_document->documentLayout(), &QAbstractTextDocumentLayout::update, this, &DocumentBinder::scheduleLayoutChanged);
    connect(m_document->documentLayout(), &QAbstractTextDocumentLayout::documentSizeChanged, this, &DocumentBinder::scheduleLayoutChanged);
    m_pendingPosition = -1;
    updateTitles();
    updateFormat();
    updateCursor();
}

/**! @brief Swap the target over to an empty document when the shown
 * document is about to be deleted.
 *
 * The TextEdit cannot be without a document, and it outlives the project's
 * documents.
 */
void DocumentBinder::releaseDocument(Document *document)
{
    if (!m_document || document != m_document) return;

    m_document->disconnect(m_target);
    m_document->disconnect(this);
    m_document->documentLayout()->disconnect(m_target);
    m_document->documentLayout()->disconnect(this);
    m_highlighter->setDocument(nullptr);
    m_document = nullptr;
    m_pendingPosition = -1;
    m_titles.clear();
    m_splitHandle.clear();
    scheduleLayoutChanged();

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
 * The document format supports a block indent from 0 to 9. Titles are not
 * indented.
 */
void DocumentBinder::changeIndent(int step)
{
    QTextCursor cursor = targetCursor();
    if (cursor.isNull()) return;

    QTextBlock block = m_document->findBlock(cursor.selectionStart());
    const QTextBlock last = m_document->findBlock(cursor.selectionEnd());
    cursor.beginEditBlock();
    while (block.isValid()) {
        if (!Document::isTitle(block)) {
            QTextCursor edit(block);
            QTextBlockFormat format = block.blockFormat();
            format.setIndent(qBound(0, format.indent() + step, 9));
            edit.setBlockFormat(format);
        }
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

/**! @brief Tell the highlighter where the target's cursor is, and update
 * the document the cursor is in.
 */
void DocumentBinder::updateCursor()
{
    QString handle;
    bool inTitle = false;
    if (m_target && m_document) {
        const int position = m_target->property("cursorPosition").toInt();
        m_highlighter->setCursorPosition(position);
        const QTextBlock block = m_document->findBlock(position);
        inTitle = Document::isTitle(block);
        handle = m_document->handleAt(position);
    }
    if (handle != m_currentHandle && handle != m_splitHandle) m_splitHandle.clear();
    if (handle == m_currentHandle && inTitle == m_inTitle) return;
    m_currentHandle = handle;
    m_inTitle = inTitle;
    emit cursorItemChanged();
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

/**! @brief Record the block of each title, and the document the cursor is
 * in, after an edit.
 */
void DocumentBinder::updateTitles()
{
    m_titles.clear();
    if (m_document) {
        for (QTextBlock block = m_document->begin(); block.isValid(); block = block.next()) {
            if (!Document::isTitle(block)) continue;
            const QString handle = Document::itemOf(block).handle;
            if (!m_titles.contains(handle)) m_titles.insert(handle, block.blockNumber());
        }
    }
    updateCursor();
    scheduleLayoutChanged();
}

/**! @brief Move the target's cursor, and drop the selection.
 */
void DocumentBinder::setCursor(int position)
{
    if (!m_target) return;
    m_target->setProperty("cursorPosition", position);
    QMetaObject::invokeMethod(m_target, "deselect");
}

/**! @brief Merge the document of a title into the one before it, and put
 * the cursor where its text starts.
 *
 * The first document has nothing to merge into. If it is empty, it is
 * removed instead, and the cursor moves to the next one.
 */
bool DocumentBinder::mergeUp(const QTextBlock &title)
{
    if (!m_project) return false;
    const QString handle = Document::itemOf(title).handle;
    if (title.position() > 0) {
        const int position = m_project->mergeDocument(handle);
        if (position >= 0) setCursor(position);
        return position >= 0;
    }
    if (!m_document->itemText(handle).isEmpty() || !m_project->deleteDocument(handle)) return false;
    enterItem(Document::itemOf(m_document->firstBlock()).handle);
    return true;
}

/**! @brief Cycle the type of the document made by the last split, from
 * scene, to scene with a hard break, to chapter, and back to scene.
 */
void DocumentBinder::upgradeSplit()
{
    if (!m_project || m_splitHandle.isEmpty() || m_splitHandle != m_currentHandle) return;
    ProjectModel *model = m_project->model();
    const int row = model->rowOf(m_splitHandle);
    if (row < 0) return;
    const QModelIndex index = model->index(row);
    if (model->data(index, ProjectModel::LevelRole).toInt() == ItemLevel::ChapterLevel) {
        model->setLevel(row, ItemLevel::SceneLevel);
    } else if (model->data(index, ProjectModel::HardBreakRole).toBool()) {
        model->setHardBreak(row, false);
        model->setLevel(row, ItemLevel::ChapterLevel);
    } else {
        model->setHardBreak(row, true);
    }
}

/**! @brief The title block of a document, or an invalid block.
 */
QTextBlock DocumentBinder::titleBlock(const QString &handle) const
{
    if (!m_document) return QTextBlock();
    const QTextBlock block = m_document->findBlockByNumber(m_titles.value(handle, -1));
    if (Document::isTitle(block) && Document::itemOf(block).handle == handle) return block;
    return m_document->titleBlock(handle);
}

/**! @brief Tell the editor that the layout has changed, once the current
 * edit is finished.
 */
void DocumentBinder::scheduleLayoutChanged()
{
    if (m_layoutPending) return;
    m_layoutPending = true;
    auto notify = [this]() {
        m_layoutPending = false;
        ++m_layoutRevision;
        emit layoutChanged();
    };
    QMetaObject::invokeMethod(this, notify, Qt::QueuedConnection);
}

} // namespace Collett
