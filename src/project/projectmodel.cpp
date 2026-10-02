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
 *
 * The title is part of the project structure, so it is saved with the
 * project, not with the document.
 */
void ProjectModel::setTitle(int row, const QString &title)
{
    Node *node = m_group ? m_group->item(row) : nullptr;
    if (!node || node->title() == title) {
        return;
    }
    node->setTitle(title);
    emit dataChanged(index(row), index(row), {Qt::DisplayRole, TitleRole});
    emit structureChanged();
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

/**! @brief Work out which rows are hidden, and which can be folded.
 *
 * A folded partition hides everything up to the next partition, and a
 * folded chapter hides the scenes up to the next chapter or partition. A
 * partition or chapter can be folded if the next document is below it.
 */
void ProjectModel::updateStructure()
{
    m_hidden.clear();
    m_foldable.clear();
    if (!m_group) {
        return;
    }

    const QList<Node *> &items = m_group->items();
    m_hidden.reserve(items.size());
    m_foldable.reserve(items.size());

    bool partitionFolded = false;
    bool chapterFolded = false;
    for (qsizetype i = 0; i < items.size(); ++i) {
        const Node *node = items.at(i);
        const ItemLevel level = node->itemLevel();
        switch (level) {
        case ItemLevel::PartitionLevel:
            m_hidden.append(false);
            partitionFolded = !node->isExpanded();
            chapterFolded = false;
            break;
        case ItemLevel::ChapterLevel:
            m_hidden.append(partitionFolded);
            chapterFolded = !node->isExpanded();
            break;
        case ItemLevel::SceneLevel:
        case ItemLevel::PageLevel:
            m_hidden.append(partitionFolded || chapterFolded);
            break;
        }
        const Node *next = (i + 1 < items.size()) ? items.at(i + 1) : nullptr;
        m_foldable.append(node->isFoldable() && next && next->itemLevel() > level);
    }
}

} // namespace Collett
