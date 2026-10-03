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

// Setters
// =======

/**! @brief Set the TextEdit item that displays the document.
 */
void DocumentBinder::setTarget(QQuickItem *target)
{
    if (m_target == target) return;
    m_target = target;
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
    m_document->documentLayout()->disconnect(m_target);
    m_highlighter->setDocument(nullptr);
    m_document = nullptr;

    if (!m_placeholder) m_placeholder = new QTextDocument(this);
    if (m_target) {
        if (auto *textDocument = m_target->property("textDocument").value<QQuickTextDocument *>()) {
            textDocument->setTextDocument(m_placeholder);
        }
    }
}

} // namespace Collett
