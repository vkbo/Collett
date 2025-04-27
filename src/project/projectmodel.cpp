/*
** Collett – Project Model Class
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

#include "collett.h"
#include "projectmodel.h"
#include "tree.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include <QMimeData>
#include <QModelIndex>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QUuid>
#include <QVariant>

using namespace Qt::Literals::StringLiterals;

namespace Collett {

// Constructor/Destructor
// ======================

ProjectModel::ProjectModel(Tree *parent) : QAbstractItemModel(parent), m_tree(parent) {
    m_root = new Node(m_tree, ItemType::InvisibleRoot, QUuid::createUuid(), "InvisibleRoot");
    m_root->setParent(this);
}

ProjectModel::~ProjectModel() {
    qDebug() << "Destructor: ProjectModel";
}

// Getters
// =======

/**!
 * @brief Get the root note of an item.
 *
 * @return Node* The root node or nullptr if invisible root.
 */
Node *ProjectModel::rootNode(Node *node) {

    Node *self = node;
    Node *root = nullptr;
    if (node) {
        while (self->parent()) {
            root = self;
            self = self->parent();
        }
    }
    return root;
}

// Public Methods
// ==============

/**!
 * @brief Pack all nodes into a JSON object.
 *
 * @param data The JSON object to populate.
 */
void ProjectModel::pack(QJsonObject &data) {
    if (m_root) m_root->pack(data);
}

/**!
 * @brief Unpack nodes from a JSON object.
 *
 * @param data The JSON object to unpack.
 */
void ProjectModel::unpack(const QJsonObject &data) {
    int skipped = 0;
    int errors = 0;
    if (data.contains("x:items"_L1) && data["x:items"_L1].isArray()) {
        for (const QJsonValue &value : data["x:items"_L1].toArray()) {
            if (value.isObject()) {
                m_root->unpack(value.toObject(), skipped, errors);
            } else {
                qWarning() << "Project root node is not a JSON object";
            }
        }
    } else {
        qWarning() << "No root nodes in project";
    }
}

// Model Access
// ============

 QModelIndex ProjectModel::index(int row, int column, const QModelIndex &parent) const {

    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    Node *parentNode;
    if (!parent.isValid()) {
        parentNode = m_root;
    } else {
        parentNode = static_cast<Node*>(parent.internalPointer());
    }

    Node *childNode = parentNode->child(row);
    if (childNode) {
        return createIndex(row, column, childNode);
    } else {
        return QModelIndex();
    }
}

QModelIndex ProjectModel::parent(const QModelIndex &index) const {

    if (!index.isValid()) {
        return QModelIndex();
    }

    Node *childNode = static_cast<Node*>(index.internalPointer());
    Node *parentNode = childNode->parent();

    if (parentNode == m_root) {
        return QModelIndex();
    } else {
        return createIndex(parentNode->row(), 0, parentNode);
    }
}

int ProjectModel::rowCount(const QModelIndex &parent) const {

    Node *parentNode;
    if (!parent.isValid()) {
        parentNode = m_root;
    } else {
        parentNode = static_cast<Node*>(parent.internalPointer());
    }
    return parentNode->childCount();
}

int ProjectModel::columnCount(const QModelIndex &parent) const {
    return 4;
}

QVariant ProjectModel::data(const QModelIndex &index, int role) const {

    if (!index.isValid()) {
        return QVariant();
    }
    Node *node = static_cast<Node*>(index.internalPointer());
    return node->data(index.column(), role);
}

Qt::ItemFlags ProjectModel::flags(const QModelIndex &index) const {

    if (!index.isValid()) {
        return Qt::NoItemFlags;
    } else {
        Node *node = static_cast<Node*>(index.internalPointer());
        return node->flags();
    }
}

/**!
 * @brief Return a list of the index of all expanded nodes.
 *
 * @return QList<QModelIndex> The list of indexes.
 */
QList<QModelIndex> ProjectModel::allExpanded() {

    QList<QModelIndex> expanded;
    for (Node *node : m_root->allChildren()) {
        if (node->isExpanded()) {
            expanded.append(createIndex(node->row(), 0, node));
        }
    }
    return expanded;
}

/**!
 * @brief Return the node at a give index, if one exists.
 *
 * @param index  The model index to look up.
 * @return Node* The node, or nullptr if it does not exist.
 */
Node *ProjectModel::nodeAtIndex(const QModelIndex &index) {
    if (index.isValid()) {
        return static_cast<Node*>(index.internalPointer());
    }
    return nullptr;
}

QModelIndex ProjectModel::indexFromHandle(const QUuid &uuid) {
    Node *node = m_tree->node(uuid);
    if (node) {
        qDebug() << "Ping!" << node->name() << node->handle();
        return createIndex(node->row(), 0, node);
    }
    return QModelIndex();
}

// Model Edit
// ==========

/**!
 * @brief Insert a child node at a given position under a parent.
 *
 * @param child  The child node to insert.
 * @param parent The parent of the node.
 * @param pos    The position of the node under the parent.
 */
void ProjectModel::insertChild(Node *child, const QModelIndex &parent, qsizetype pos) {

    Node *node;
    if (parent.isValid()) {
        node = static_cast<Node*>(parent.internalPointer());
    } else {
        node = m_root;
    }
    int row = qMin(qMax(pos, 0), node->childCount());
    emit beginInsertRows(parent, row, row);
    node->addChild(child, row);
    emit endInsertRows();
}

/**!
 * @brief Remove a child from a node index.
 *
 * This removes the child from the parent and returns a pointer to it.
 * The node is also removed from the nodes record in the Tree class, which is
 * handled internall in the parent node.
 *
 * @param parent The parent index of the child.
 * @param pos    The position of the child in the parent node.
 * @return Node* The remmoved child node.
 */
Node *ProjectModel::removeChild(const QModelIndex &parent, qsizetype pos) {

    Node *node;
    if (parent.isValid()) {
        node = static_cast<Node*>(parent.internalPointer());
    } else {
        node = m_root;
    }
    if (pos >= 0 && pos < node->childCount()) {
        emit beginRemoveRows(parent, pos, pos);
        Node *child = node->takeChild(pos);
        emit endRemoveRows();
        return child;
    }
    return nullptr;
}

/**!
 * @brief Move a list of indexes to a new parent node.
 *
 * The list of indexes are moved to the new location. If a child and a parent
 * are both selected, only the parent is moved and the child just follows
 * along. The internal meta data of the nodes are updated by the parent node
 * object so that its class and item level info is consistent with its new
 * location in the project.
 *
 * @param indexes A list of indexes to be moved.
 * @param parent  The parent index to move the indexes to.
 * @param pos     The position under the parent index to move the indexes to.
 */
void ProjectModel::multiMove(const QModelIndexList &indexes, const QModelIndex &parent, qsizetype pos) {
    if (!parent.isValid()) return;

    // This is a two pass process. First we only select unique non-root items
    // for move, then we do a second pass and only move those items that don't
    // have a parent also scheduled for moving or have already been moved.
    // Child items are moved with the parent.

    QSet<QUuid> handles;
    QList<Node*> pruned;
    for (QModelIndex index : indexes) {
        if (index.isValid()) {
            Node *node = static_cast<Node*>(index.internalPointer());
            if (node && !node->isRootType() && !handles.contains(node->handle())) {
                pruned.prepend(node);  // Built in reverse order
                handles.insert(node->handle());
            }
        }
    }
    qDebug() << indexes;

    for (Node *mNode : pruned) {
        Node *pNode = mNode->parent();
        if (pNode && !handles.contains(pNode->handle())) {
            QModelIndex index = createIndex(mNode->row(), 0, mNode);
            Node *tNode = this->removeChild(index.parent(), index.row());
            qDebug() << tNode;
            if (tNode) {
                this->insertChild(tNode, parent, pos);
                for (Node *cNode : mNode->allChildren()) {
                    cNode->updateValues();
                }
            }
        }
    }
}

/**!
 * @brief Add a root folder relative to the selected index.
 *
 * Root folders are always added after the root folder ancestor of the selected
 * index, or by default at the end of the project tree.
 *
 * @param name      The name of the new root folder.
 * @param itemClass The class of the new root folder.
 * @param selected  The selected index to add the root folder relative to.
 * @return Node*    The root folder node.
 */
Node *ProjectModel::addRoot(QString name, ItemClass itemClass, const QModelIndex &selected) {

    qsizetype pos = m_root->childCount();
    if (selected.isValid()) {
        Node *sNode = this->rootNode(static_cast<Node*>(selected.internalPointer()));
        if (sNode) pos = sNode->row() + 1;
    }

    Node *child = m_root->createRoot(QUuid::createUuid(), name, itemClass);
    child->setActive(true);
    this->insertChild(child, QModelIndex(), pos);
    return child;
}

/**!
 * @brief Add a folder relative to the selected index.
 *
 * The folder is added as a child if the selected index is a root folder or an
 * expanded folder with other child items. Otherwise, the folder is added next
 * to the selected item as a sibling.
 *
 * @param name     The name of the new folder.
 * @param selected The selected index to add the folder relative to.
 * @return Node*   The folder node or nullptr if failed.
 */
Node *ProjectModel::addFolder(QString name, const QModelIndex &selected) {

    if (!selected.isValid()) return nullptr;

    Node *sNode = static_cast<Node*>(selected.internalPointer());
    if (!sNode) return nullptr;

    // The default behaviour is to make the new item a sibling of the selected item
    QModelIndex parent = selected.parent();
    qsizetype pos = selected.row() + 1;

    if (sNode->isRootType()) {
        // For root folders, it must be a child
        parent = selected;
        pos = sNode->childCount();
    } else if (sNode->isFolderType() && sNode->isExpanded() && sNode->childCount() > 0) {
        // If the node is an expanded folder with children, add the new folder as a child
        parent = selected;
        pos = sNode->childCount();
    }

    if (parent.isValid()) {
        Node *nNode = static_cast<Node*>(parent.internalPointer());
        if (nNode) {
            Node *child = nNode->createFolder(QUuid::createUuid(), name);
            child->setActive(true);
            this->insertChild(child, parent, pos);
            return child;
        }
    }
    return nullptr;
}

/**!
 * @brief Add a file relative to the selected index.
 *
 * New files are by default added next to the selected item as a sibling, with
 * the following exceptions:
 *
 * 1. If the selected index is a root or folder, the file is added as a child
 *    at the end of the list of children.
 *
 * 2. If the parent of the selected index is of similar or higher structural
 *    level than the selected index, then the new file is added as a sibling of
 *    the parent instead of the selected.
 *
 * 3. If the selected index has children and the new file is of lower
 *    structural level or is a note, the new file is added as a child of the
 *    selected index.
 *
 * @param name      The name of the new file.
 * @param itemLevel The item level of the file.
 * @param selected  The selected index to add the file relative to.
 * @return Node*    The file node or nullptr if failed.
 */
Node *ProjectModel::addFile(QString name, ItemLevel itemLevel, const QModelIndex &selected) {

    if (!selected.isValid()) return nullptr;

    Node *sNode = static_cast<Node*>(selected.internalPointer());
    if (!sNode) return nullptr;

    // The default behaviour is to make the new item a sibling of the selected item
    QModelIndex parent = selected.parent();
    qsizetype pos = selected.row() + 1;

    if (sNode->isRootType() || sNode->isFolderType()) {
        // Always add as a direct child of any folder
        parent = selected;
        pos = sNode->childCount();

    } else if (sNode->isFileType()) {
        Node *pNode = sNode->parent();

        int hLevel = static_cast<int>(itemLevel);
        int sLevel = static_cast<int>(sNode->itemLevel());
        int pLevel = static_cast<int>(pNode->itemLevel());

        bool isNote = false;
        if (itemLevel == ItemLevel::NoteLevel) {
            isNote = true;
            sLevel = 0;  // Here we treat selected notes as level 0
        }

        if (pNode && pNode->isFileType() && pLevel >= hLevel && sLevel > hLevel) {
            // If the selected item's is a higher level than the new item, and
            // the parent level is equal or higher, we make it a sibling of the parent
            parent = parent.parent();
            pos = pNode->row() + 1;
        }

        if (sNode->childCount() > 0 && ((sLevel > 0 && sLevel < hLevel) || isNote)) {
            // If the selected item already has child nodes and
            // has a lower level or is a note, we make the new item a child
            parent = selected;
            pos = sNode->childCount();
        }
    }

    if (parent.isValid()) {
        Node *nNode = static_cast<Node*>(parent.internalPointer());
        if (nNode) {
            Node *child = nNode->createFile(QUuid::createUuid(), name, itemLevel);
            child->setActive(true);
            this->insertChild(child, parent, pos);
            return child;
        }
    }
    return nullptr;
}

// Drag and Drop
// =============

QStringList ProjectModel::mimeTypes() const {
    return {PROJECT_ITEM_MIME};
}

QMimeData *ProjectModel::mimeData(const QModelIndexList &indexes) const {

    QMimeData *mimeData = new QMimeData;
    QList<QByteArray> handles;

    for (QModelIndex index : indexes) {
        if (index.isValid() && index.column() == 0) {
            Node *node = static_cast<Node*>(index.internalPointer());
            handles << node->handle().toByteArray(QUuid::WithoutBraces);
        }
    }

    mimeData->setData(PROJECT_ITEM_MIME, handles.join(";"));
    return mimeData;
}

Qt::DropActions ProjectModel::supportedDropActions() const {
    return Qt::DropAction::MoveAction;
}

bool ProjectModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const {

    if (parent.isValid() && parent.internalPointer() != m_root) {
        return data->hasFormat(PROJECT_ITEM_MIME) && action == Qt::MoveAction;
    }
    return false;
}

bool ProjectModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) {

    if (this->canDropMimeData(data, action, row, column, parent)) {
        QModelIndexList indexes;
        for (const QUuid handle : decodeMimeHandles(data)) {
            indexes << this->indexFromHandle(handle);
        }
        this->multiMove(indexes, parent, row);
        return true;
    }
    return false;
}

// Static Methods
// ==============

/**!
 * @brief Static method to decode handles from mime data.
 *
 * @param mimeData      The mimedata object.
 * @return QList<QUuid> A list of handle UUIDs.
 */
QList<QUuid> ProjectModel::decodeMimeHandles(const QMimeData *mimeData) {

    QList<QUuid> handles;
    for (QByteArray handle : mimeData->data(PROJECT_ITEM_MIME).split(';')) {
        handles << QUuid(handle);
    }
    return handles;
}

} // namespace Collett
