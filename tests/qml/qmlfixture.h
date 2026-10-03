/*
** Collett - QML Test Fixture
** ==========================
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

#pragma once

#include "collett.h"
#include "document.h"
#include "documentbinder.h"
#include "icons.h"
#include "project.h"
#include "projectmodel.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTemporaryDir>
#include <QTextBlock>
#include <QTextCursor>
#include <QtQml/QQmlExtensionPlugin>
#include <QtTest>

// The QML module is linked statically
Q_IMPORT_QML_PLUGIN(CollettPlugin)

namespace Collett {

/**! @brief Set up the application the same way as the main function, and
 * keep settings away from a real install. Called once, from initTestCase.
 */
inline void initQmlTests()
{
    QCoreApplication::setOrganizationName("CollettTest");
    QCoreApplication::setApplicationName("tst_qml");
    QQuickWindow::setTextRenderType(QQuickWindow::NativeTextRendering);
}

/**! @brief A project in a temporary folder, shown in the main window.
 *
 * Documents are added before the window is loaded. The project is created
 * with a title page, so the first added document is at row 1.
 */
class QmlFixture
{
public:
    QmlFixture() = default;
    ~QmlFixture()
    {
        delete m_engine;
    }

    Project project;

    /**! @brief Create the project, with a title page.
     */
    bool create()
    {
        return m_dir.isValid() && project.createProject(m_dir.path(), "Test Novel").isEmpty();
    }

    /**! @brief Add a document at the end, with one paragraph per line of
     * text, or no text. Returns its handle.
     */
    QString addDocument(ItemLevel level, const QString &title, const QString &text)
    {
        Document::Item item;
        item.handle = project.tree()->newHandle();
        item.title = title;
        item.level = level;
        QJsonArray content;
        if (!text.isEmpty()) {
            for (const QString &line : text.split('\n'))
                content.append(QJsonObject({{"u:fmt", "p"}, {"u:txt", "t|" + line}}));
        }
        doc()->appendItem(item, content);
        return item.handle;
    }

    /**! @brief Load the main window and wait for it to be shown.
     */
    bool load()
    {
        m_engine = new QQmlApplicationEngine();
        m_engine->addImageProvider("icons", new Icons("lucide"));
        m_engine->setInitialProperties({{"project", QVariant::fromValue(&project)}});
        m_engine->loadFromModule("Collett", "Main");
        if (m_engine->rootObjects().isEmpty()) return false;
        window = qobject_cast<QQuickWindow *>(m_engine->rootObjects().first());
        if (!window || !QTest::qWaitForWindowExposed(window)) return false;

        // An open project moves the cursor to the last edited document once
        // the view is laid out, which must happen before a test moves it
        return !project.isValid() || QTest::qWaitFor([this]() { return focusHandle() == project.lastEditedHandle(); });
    }

    QQuickWindow *window = nullptr;
    QQmlApplicationEngine *engine() const { return m_engine; }

    // Model
    Document *doc() const { return project.editorDocument(); }
    ProjectModel *model() const { return project.model(); }
    int rows() const { return model()->rowCount(); }
    QVariant value(int row, int role) const { return model()->data(model()->index(row), role); }
    QString handle(int row) const { return value(row, ProjectModel::HandleRole).toString(); }
    QString text(int row) const { return doc()->itemText(handle(row)); }

    /**! @brief The handles in row order.
     */
    QStringList order() const
    {
        QStringList handles;
        for (int i = 0; i < rows(); ++i)
            handles.append(handle(i));
        return handles;
    }

    // Items
    QQuickItem *item(const QString &name) const
    {
        return window->findChild<QQuickItem *>(name);
    }

    /**! @brief The item at a row of a list view, scrolled into view.
     */
    QQuickItem *itemAt(const QString &list, int row) const
    {
        QQuickItem *view = item(list);
        QMetaObject::invokeMethod(view, "positionViewAtIndex", Q_ARG(int, row), Q_ARG(int, 4)); // Contain
        QQuickItem *found = nullptr;
        QMetaObject::invokeMethod(view, "itemAtIndex", Q_RETURN_ARG(QQuickItem *, found), Q_ARG(int, row));
        return found;
    }
    QQuickItem *listItem(int row) const { return itemAt("projectList", row); }

    // Editor
    QQuickItem *textEdit() const { return item("textEdit"); }
    DocumentBinder *binder() const { return item("editor")->property("binder").value<DocumentBinder *>(); }

    /**! @brief The position in the editor's text of a position in the text
     * of a document.
     */
    int textPosition(int row, int position) const { return binder()->itemPosition(handle(row)) + position; }

    /**! @brief The window position of a position in the text of a document.
     */
    QPoint pointInText(int row, int position) const
    {
        QMetaObject::invokeMethod(item("editor"), "showPosition", Q_ARG(int, textPosition(row, position)));
        QRectF rect;
        QMetaObject::invokeMethod(textEdit(), "positionToRectangle", Q_RETURN_ARG(QRectF, rect), Q_ARG(int, textPosition(row, position)));
        return textEdit()->mapToScene(rect.center()).toPoint();
    }

    /**! @brief The type label above the title of a document, scrolled into
     * view, once it is in place. The labels are made by a repeater, so they
     * are found in the item tree, not the object tree.
     */
    QQuickItem *typeLabel(int row) const
    {
        QMetaObject::invokeMethod(item("editor"), "showPosition", Q_ARG(int, doc()->titleBlock(handle(row)).position()));
        QList<QQuickItem *> items = {window->contentItem()};
        while (!items.isEmpty()) {
            QQuickItem *at = items.takeFirst();
            if (at->objectName() == "typeLabel" && at->parentItem()->property("handle").toString() == handle(row)) {
                // The labels follow the layout once the editor has its size
                QQuickItem *decoration = at->parentItem();
                QTest::qWaitFor([this, decoration, row]() { return decoration->property("titleRect").toRectF() == binder()->titleRect(handle(row)); });
                return at;
            }
            items.append(at->childItems());
        }
        return nullptr;
    }

    /**! @brief The centre of an item, or a point a fraction down it, in
     * window coordinates.
     */
    QPoint pointIn(QQuickItem *target, qreal fraction = 0.5) const
    {
        return target->mapToScene(QPointF(target->width() / 2, target->height() * fraction)).toPoint();
    }

    // Focus
    /**! @brief The handle of the document with the cursor, while the editor
     * has the focus.
     */
    QString focusHandle() const
    {
        return window->activeFocusItem() == textEdit() ? binder()->currentHandle() : QString();
    }

    /**! @brief Where the cursor is in its document: "title" or "text".
     */
    QString focusPart() const
    {
        if (window->activeFocusItem() != textEdit()) return QString();
        return binder()->inTitle() ? "title" : "text";
    }

    /**! @brief The cursor position in the title or the text of its document.
     */
    int focusCursor() const
    {
        if (window->activeFocusItem() != textEdit()) return -1;
        const int position = textEdit()->property("cursorPosition").toInt();
        const QTextBlock block = doc()->findBlock(position);
        if (Document::isTitle(block)) return position - block.position();
        return position - binder()->itemPosition(binder()->currentHandle());
    }

    /**! @brief Type text, one key at a time.
     */
    void type(const QString &text) const
    {
        for (const QChar c : text)
            QTest::keyClick(window, c.toLatin1());
    }

    /**! @brief Put the cursor in the text of a document.
     */
    void enterText(int row, int position) const
    {
        binder()->enterText(handle(row), position);
        textEdit()->forceActiveFocus();
    }

    /**! @brief Put the cursor at the start of the title of a document.
     */
    void enterTitle(int row) const
    {
        binder()->enterTitle(handle(row));
        textEdit()->forceActiveFocus();
    }

private:
    QTemporaryDir m_dir;
    QQmlApplicationEngine *m_engine = nullptr;
};

} // namespace Collett
