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

#include <QAbstractTextDocumentLayout>
#include <QQuickTextDocument>
#include <QSizeF>

namespace Collett {

// Constructor/Destructor
// ======================

DocumentBinder::DocumentBinder(QObject *parent) : QObject(parent) {}

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
    m_project = project;
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
    m_document->setDocumentMargin(m_target->property("textMargin").toReal());
    textDocument->setTextDocument(m_document);

    connect(m_document, SIGNAL(contentsChange(int, int, int)), m_target, SLOT(q_contentsChange(int, int, int)));
    connect(m_document->documentLayout(), SIGNAL(updateBlock(QTextBlock)), m_target, SLOT(invalidateBlock(QTextBlock)));
    connect(m_document, SIGNAL(undoAvailable(bool)), m_target, SIGNAL(canUndoChanged()));
    connect(m_document, SIGNAL(redoAvailable(bool)), m_target, SIGNAL(canRedoChanged()));
}

} // namespace Collett
