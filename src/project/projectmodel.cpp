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

#include "projectmodel.h"

#include <QModelIndex>
#include <QSet>
#include <QVariant>

namespace Collett {

// Constructor/Destructor
// ======================

ProjectModel::ProjectModel(QObject *parent) : QAbstractListModel(parent) {}

ProjectModel::~ProjectModel()
{
    qDebug() << "Destructor: ProjectModel";
}

// Model Interface
// ===============

int ProjectModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !m_group) {
        return 0;
    }
    return int(m_group->count());
}

QVariant ProjectModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_group) {
        return QVariant();
    }

    const Node *node = m_group->item(index.row());
    if (!node) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
    case TitleRole: return node->title();
    case HandleRole: return node->handle();
    case LevelRole: return int(node->itemLevel());
    case WordsRole: return node->counts().words;
    case ExpandedRole: return node->isExpanded();
    case FoldableRole: return m_foldable.value(index.row(), false);
    case HiddenRole: return m_hidden.value(index.row(), false);
    case NumberRole: return m_numbers.value(index.row(), 0);
    case HardBreakRole: return node->hasHardBreak();
    case NumberedRole: return node->isNumbered();
    case ChapterNumberRole: return m_chapterNumbers.value(index.row(), 0);
    }
    return QVariant();
}

QHash<int, QByteArray> ProjectModel::roleNames() const
{
    return {
        {HandleRole, "handle"},
        {TitleRole, "title"},
        {LevelRole, "level"},
        {WordsRole, "words"},
        {ExpandedRole, "expanded"},
        {FoldableRole, "foldable"},
        {HiddenRole, "hidden"},
        {NumberRole, "number"},
        {HardBreakRole, "hardBreak"},
        {NumberedRole, "numbered"},
        {ChapterNumberRole, "chapterNumber"},
    };
}

// Methods
// =======

/**! @brief Show the documents of a group, or nothing if it is nullptr.
 */
void ProjectModel::setGroup(Group *group)
{
    beginResetModel();
    m_group = group;
    updateStructure();
    endResetModel();
}

/**! @brief Fold or unfold a partition or chapter.
 *
 * Scenes cannot be folded, and neither can a partition or chapter with
 * nothing under it.
 */
void ProjectModel::toggleExpanded(int row)
{
    if (!m_group || !m_foldable.value(row, false)) {
        return;
    }

    Node *node = m_group->item(row);
    node->setExpanded(!node->isExpanded());

    const QList<bool> oldHidden = m_hidden;
    updateStructure();

    emit dataChanged(index(row), index(row), {ExpandedRole});
    emit structureChanged();
    for (int i = 0; i < m_hidden.size(); ++i) {
        if (m_hidden.at(i) != oldHidden.value(i)) {
            emit dataChanged(index(i), index(i), {HiddenRole});
        }
    }
}

/**! @brief Set the title of a document.
 */
void ProjectModel::setTitle(int row, const QString &title)
{
    Node *node = m_group ? m_group->item(row) : nullptr;
    if (!node || node->title() == title) {
        return;
    }
    if (m_document) {
        m_document->setItemTitle(node->handle(), title);
        return;
    }
    node->setTitle(title);
    emit dataChanged(index(row), index(row), {Qt::DisplayRole, TitleRole});
    emit structureChanged();
}

/**! @brief Change the level of a document.
 *
 * Changing a level can change the numbering, and what can be folded or is
 * hidden, for the documents that follow, so all rows are refreshed.
 */
void ProjectModel::setLevel(int row, int level)
{
    Node *node = m_group ? m_group->item(row) : nullptr;
    if (!node || level < ItemLevel::PartitionLevel || level > ItemLevel::PageLevel || node->itemLevel() == level) {
        return;
    }
    if (m_document) {
        Document::Item item = itemOf(node);
        item.level = ItemLevel(level);
        m_document->setItemValues(item);
        return;
    }
    node->setLevel(ItemLevel(level));
    node->setExpanded(true);
    emit dataChanged(index(row), index(row), {LevelRole, ExpandedRole, HardBreakRole, NumberedRole});
    refreshStructure();
}

/**! @brief Set whether a scene has a hard break before it.
 */
void ProjectModel::setHardBreak(int row, bool state)
{
    Node *node = m_group ? m_group->item(row) : nullptr;
    if (!node || node->itemLevel() != ItemLevel::SceneLevel || node->hasHardBreak() == state) {
        return;
    }
    if (m_document) {
        Document::Item item = itemOf(node);
        item.hardBreak = state;
        m_document->setItemValues(item);
        return;
    }
    node->setHardBreak(state);
    emit dataChanged(index(row), index(row), {HardBreakRole});
    emit structureChanged();
}

/**! @brief Set whether a chapter is numbered.
 *
 * An unnumbered chapter is left out of the numbering, so the numbers of the
 * chapters after it change too.
 */
void ProjectModel::setNumbered(int row, bool state)
{
    Node *node = m_group ? m_group->item(row) : nullptr;
    if (!node || node->itemLevel() != ItemLevel::ChapterLevel || node->isNumbered() == state) {
        return;
    }
    if (m_document) {
        Document::Item item = itemOf(node);
        item.numbered = state;
        m_document->setItemValues(item);
        return;
    }
    node->setNumbered(state);
    emit dataChanged(index(row), index(row), {NumberedRole});
    refreshStructure();
}

/**! @brief Set the text counts of a document.
 *
 * Only the words and characters are saved, so a change to the other counts
 * alone, like the paragraphs after the counts are read from a file, is not a
 * change to the structure.
 */
void ProjectModel::setCounts(int row, const TextCounts &counts)
{
    Node *node = m_group ? m_group->item(row) : nullptr;
    if (!node || node->counts() == counts) {
        return;
    }
    const TextCounts old = node->counts();
    node->setCounts(counts);
    if (old.words == counts.words && old.characters == counts.characters) {
        return;
    }
    emit dataChanged(index(row), index(row), {WordsRole});
    emit structureChanged();
}

/**! @brief The number of rows that move together with a row: the row, and
 * the hidden rows of a folded partition or chapter.
 */
int ProjectModel::blockSize(int row) const
{
    if (!m_group || row < 0 || row >= m_group->count()) {
        return 0;
    }
    int end = row + 1;
    while (end < m_hidden.size() && m_hidden.at(end)) {
        ++end;
    }
    return end - row;
}

/**! @brief Move a block of rows to before another row.
 *
 * Documents that were in view and would end up hidden by a folded partition
 * or chapter unfold it, so a move never hides anything.
 */
bool ProjectModel::moveBlock(int row, int count, int before)
{
    const int total = m_group ? int(m_group->count()) : 0;
    if (count < 1 || row < 0 || row + count > total || before < 0 || before > total || (before >= row && before <= row + count)) {
        return false;
    }

    QSet<const Node *> shown;
    for (int i = 0; i < total; ++i) {
        if (!m_hidden.value(i)) shown.insert(m_group->item(i));
    }

    if (m_document) {
        // The model follows the document as it changes
        m_document->moveItems(row, count, before);
    } else {
        beginMoveRows(QModelIndex(), row, row + count - 1, QModelIndex(), before);
        const int target = before > row ? before - count : before;
        QList<Node *> moved;
        for (int i = 0; i < count; ++i) {
            moved.append(m_group->takeItem(row));
        }
        for (int i = 0; i < count; ++i) {
            m_group->insertItem(target + i, moved.at(i));
        }
        updateStructure();
        endMoveRows();
    }

    for (int r = 0; r < total; ++r) {
        if (!shown.contains(m_group->item(r))) continue;
        for (int i = r - 1; i >= 0 && m_hidden.value(r); --i) {
            Node *node = m_group->item(i);
            if (m_foldable.value(i) && !node->isExpanded()) {
                node->setExpanded(true);
                updateStructure();
            }
        }
    }
    refreshStructure();
    return true;
}

/**! @brief Insert a new document at a row.
 *
 * The model takes the node into its group, which then owns it.
 */
void ProjectModel::insertNode(int row, Node *node)
{
    if (!m_group || !node) {
        return;
    }
    row = qBound(0, row, int(m_group->count()));
    beginInsertRows(QModelIndex(), row, row);
    m_group->insertItem(row, node);
    updateStructure();
    endInsertRows();
    refreshStructure();
}

/**! @brief Remove a document from the group, and return it.
 *
 * The caller owns the returned node.
 */
Node *ProjectModel::takeNode(int row)
{
    if (!m_group || row < 0 || row >= m_group->count()) {
        return nullptr;
    }
    beginRemoveRows(QModelIndex(), row, row);
    Node *node = m_group->takeItem(row);
    updateStructure();
    endRemoveRows();
    refreshStructure();
    return node;
}

/**! @brief Make the rows match the documents of the text document.
 *
 * Rows of documents that are gone are removed and handed to dispose, rows
 * are moved into the new order, and rows for new documents are made with
 * create. Each row then takes the title, level and break settings of its
 * document.
 */
void ProjectModel::sync(const QList<Document::Item> &items, const std::function<Node *(const Document::Item &)> &create, const std::function<void(Node *)> &dispose)
{
    if (!m_group) {
        return;
    }

    QSet<QString> wanted;
    for (const Document::Item &item : items) {
        wanted.insert(item.handle);
    }
    bool changed = false;
    for (int row = int(m_group->count()) - 1; row >= 0; --row) {
        if (wanted.contains(m_group->item(row)->handle())) continue;
        beginRemoveRows(QModelIndex(), row, row);
        Node *node = m_group->takeItem(row);
        updateStructure();
        endRemoveRows();
        dispose(node);
        changed = true;
    }

    for (int row = 0; row < items.size(); ++row) {
        const Document::Item &item = items.at(row);
        if (row >= m_group->count() || m_group->item(row)->handle() != item.handle) {
            int from = -1;
            for (int i = row + 1; i < m_group->count(); ++i) {
                if (m_group->item(i)->handle() == item.handle) {
                    from = i;
                    break;
                }
            }
            if (from >= 0) {
                beginMoveRows(QModelIndex(), from, from, QModelIndex(), row);
                m_group->insertItem(row, m_group->takeItem(from));
                updateStructure();
                endMoveRows();
            } else {
                beginInsertRows(QModelIndex(), row, row);
                m_group->insertItem(row, create(item));
                updateStructure();
                endInsertRows();
            }
            changed = true;
        }

        Node *node = m_group->item(row);
        QList<int> roles;
        if (node->title() != item.title) {
            node->setTitle(item.title);
            roles << Qt::DisplayRole << TitleRole;
        }
        if (node->itemLevel() != item.level) {
            node->setLevel(item.level);
            node->setExpanded(true);
            roles << LevelRole << ExpandedRole;
        }
        if (node->hasHardBreak() != (item.hardBreak && item.level == ItemLevel::SceneLevel)) {
            node->setHardBreak(item.hardBreak);
            roles << HardBreakRole;
        } else {
            node->setHardBreak(item.hardBreak);
        }
        if (node->isNumbered() != (item.numbered || item.level != ItemLevel::ChapterLevel)) {
            node->setNumbered(item.numbered);
            roles << NumberedRole;
        } else {
            node->setNumbered(item.numbered);
        }
        if (!roles.isEmpty()) {
            emit dataChanged(index(row), index(row), roles);
            changed = true;
        }
    }

    if (changed) {
        refreshStructure();
    }
}

/**! @brief The row of a document, or -1 if it is not in the group.
 */
int ProjectModel::rowOf(const QString &handle) const
{
    if (m_group) {
        const QList<Node *> &items = m_group->items();
        for (qsizetype i = 0; i < items.size(); ++i) {
            if (items.at(i)->handle() == handle) return int(i);
        }
    }
    return -1;
}

// Private Methods
// ===============

/**! @brief Work out which rows are hidden, which can be folded, and the
 * numbering.
 *
 * A folded partition hides everything up to the next partition, and a
 * folded chapter hides the scenes up to the next chapter or partition. A
 * partition or chapter can be folded if the next document is below it.
 * Chapters are numbered through the whole group, except those set to be
 * unnumbered, and scenes from 1 in each chapter or partition. Partitions
 * and pages are not numbered. Each document also gets the number of the
 * chapter it is in, which is 0 outside a chapter or in an unnumbered one.
 */
void ProjectModel::updateStructure()
{
    m_hidden.clear();
    m_foldable.clear();
    m_numbers.clear();
    m_chapterNumbers.clear();
    if (!m_group) {
        return;
    }

    const QList<Node *> &items = m_group->items();
    m_hidden.reserve(items.size());
    m_foldable.reserve(items.size());
    m_numbers.reserve(items.size());
    m_chapterNumbers.reserve(items.size());

    bool partitionFolded = false;
    bool chapterFolded = false;
    int chapterCount = 0;
    int sceneCount = 0;
    int chapterNumber = 0;
    for (qsizetype i = 0; i < items.size(); ++i) {
        const Node *node = items.at(i);
        const ItemLevel level = node->itemLevel();
        switch (level) {
        case ItemLevel::PartitionLevel:
            m_hidden.append(false);
            m_numbers.append(0);
            partitionFolded = !node->isExpanded();
            chapterFolded = false;
            sceneCount = 0;
            chapterNumber = 0;
            break;
        case ItemLevel::ChapterLevel:
            m_hidden.append(partitionFolded);
            m_numbers.append(node->isNumbered() ? ++chapterCount : 0);
            chapterFolded = !node->isExpanded();
            sceneCount = 0;
            chapterNumber = m_numbers.last();
            break;
        case ItemLevel::SceneLevel:
            m_hidden.append(partitionFolded || chapterFolded);
            m_numbers.append(++sceneCount);
            break;
        case ItemLevel::PageLevel:
            m_hidden.append(partitionFolded || chapterFolded);
            m_numbers.append(0);
            break;
        }
        m_chapterNumbers.append(chapterNumber);
        const Node *next = (i + 1 < items.size()) ? items.at(i + 1) : nullptr;
        m_foldable.append(node->isFoldable() && next && next->itemLevel() > level);
    }
}

/**! @brief Recompute the structure and notify the views of all rows.
 *
 * Used after a change that can affect the documents that follow it. The
 * views only update what has actually changed.
 */
void ProjectModel::refreshStructure()
{
    updateStructure();
    if (m_group && m_group->count() > 0) {
        emit dataChanged(index(0), index(int(m_group->count()) - 1), {ExpandedRole, FoldableRole, HiddenRole, NumberRole, ChapterNumberRole});
    }
    emit structureChanged();
}

/**! @brief The values of a node, as the document holds them.
 */
Document::Item ProjectModel::itemOf(const Node *node) const
{
    Document::Item item;
    item.handle = node->handle();
    item.title = node->title();
    item.level = node->itemLevel();
    item.hardBreak = node->hasHardBreak();
    item.numbered = node->isNumbered();
    return item;
}

} // namespace Collett
