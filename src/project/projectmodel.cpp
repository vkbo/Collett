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

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include <QModelIndex>
#include <QString>
#include <QUuid>
#include <QVariant>

using namespace Qt::Literals::StringLiterals;

namespace Collett {

// Constructor/Destructor
// ======================

ProjectModel::ProjectModel(QObject *parent) : QAbstractItemModel(parent) {
    m_root = new Node(ItemType::InvisibleRoot, QUuid::createUuid(), "InvisibleRoot");
}

ProjectModel::~ProjectModel() {
    qDebug() << "Destructor: ProjectModel";
}

// Public Methods
// ==============

void ProjectModel::pack(QJsonObject &data) {
    if (m_root) m_root->pack(data);
}

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
        return QAbstractItemModel::flags(index);
    }
}

QList<QModelIndex> ProjectModel::allExpanded() {
    QList<QModelIndex> expanded;
    for (Node *node : m_root->allChildren()) {
        if (node->isExpanded()) {
            expanded.append(createIndex(node->row(), 0, node));
        }
    }
    return expanded;
}

Node *ProjectModel::nodeAtIndex(const QModelIndex &index) {
    if (index.isValid()) {
        return static_cast<Node*>(index.internalPointer());
    }
    return nullptr;
}

// Model Edit
// ==========

void ProjectModel::insertChild(Node *child, const QModelIndex &parent, qsizetype pos) {

    Node *parentNode;
    if (!parent.isValid()) {
        parentNode = m_root;
    } else {
        parentNode = static_cast<Node*>(parent.internalPointer());
    }
    int row = qMin(qMax(pos, 0), parentNode->childCount());
    emit beginInsertRows(parent, row, row);
    parentNode->addChild(child, row);
    emit endInsertRows();
}

Node *ProjectModel::addRoot(QString name, ItemClass itemClass, const QModelIndex &relative) {
    return nullptr;
}

Node *ProjectModel::addFolder(QString name, const QModelIndex &relative) {
    if (!relative.isValid()) return nullptr;

    Node *node = static_cast<Node*>(relative.internalPointer());
    qsizetype pos = -1;
    QModelIndex parent;
    if (node->itemType() == ItemType::RootType) {
        parent = relative;
        pos = node->childCount();
    } else {
        node = node->parent();
        parent = relative.parent();
        pos = relative.row() + 1;
    }

    if (node && parent.isValid()) {
        int row = qMin(qMax(pos, 0), node->childCount());
        emit beginInsertRows(parent, row, row);
        Node *child = node->addFolder(QUuid::createUuid(), name, row);
        emit endInsertRows();
        return child;
    }
    return nullptr;
}

Node *ProjectModel::addFile(QString name, ItemLevel itemLevel, const QModelIndex &parent, qsizetype pos) {
    return nullptr;
}


} // namespace Collett
