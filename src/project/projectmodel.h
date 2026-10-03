/*
** Collett - Project Model Class
** =============================
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
#include "document.h"
#include "group.h"

#include <QAbstractListModel>
#include <QByteArray>
#include <QHash>
#include <QList>
#include <QModelIndex>
#include <QPointer>
#include <QString>
#include <QVariant>
#include <QtQml/qqmlregistration.h>

#include <functional>

namespace Collett {

/**! @brief A list model of the documents in one project group.
 *
 * The documents are listed in reading order. Folding a partition or chapter
 * does not remove rows, it marks the rows it covers as hidden, so the view
 * can animate them in and out.
 *
 * When the group's text document is set, it holds the structure: changes to
 * the titles, levels, breaks and order are made to the document, which can
 * undo them, and the model follows the document through sync().
 */
class ProjectModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("The project model is provided by the project")

public:
    enum Roles
    {
        HandleRole = Qt::UserRole + 1,
        TitleRole,
        LevelRole,
        WordsRole,
        ExpandedRole,
        FoldableRole,
        HiddenRole,
        NumberRole,
        HardBreakRole,
        NumberedRole,
        ChapterNumberRole,
    };
    Q_ENUM(Roles)

    explicit ProjectModel(QObject *parent = nullptr);
    ~ProjectModel();

    // Model Interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Methods
    Group *group() const { return m_group; };
    void setGroup(Group *group);
    void setDocument(Document *document) { m_document = document; };
    void sync(const QList<Document::Item> &items, const std::function<Node *(const Document::Item &)> &create, const std::function<void(Node *)> &dispose);
    Q_INVOKABLE void toggleExpanded(int row);
    Q_INVOKABLE int rowOf(const QString &handle) const;
    Q_INVOKABLE void setTitle(int row, const QString &title);
    Q_INVOKABLE void setLevel(int row, int level);
    Q_INVOKABLE void setHardBreak(int row, bool state);
    Q_INVOKABLE void setNumbered(int row, bool state);
    Q_INVOKABLE int blockSize(int row) const;
    Q_INVOKABLE bool moveBlock(int row, int count, int before);
    void setCounts(int row, const TextCounts &counts);
    void insertNode(int row, Node *node);
    Node *takeNode(int row);

signals:
    void structureChanged();

private:
    Group *m_group = nullptr;
    QPointer<Document> m_document;
    QList<bool> m_hidden;
    QList<bool> m_foldable;
    QList<int> m_numbers;
    QList<int> m_chapterNumbers;

    void updateStructure();
    void refreshStructure();
    Document::Item itemOf(const Node *node) const;
};
} // namespace Collett
