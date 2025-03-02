/*
** Collett – Project Node Class
** ============================
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

#include "node.h"
#include "theme.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QVariant>
#include <QUuid>

using namespace Qt::Literals::StringLiterals;

namespace Collett {

// Constructor/Destructor
// ======================

Node::Node(ItemType itemType, ItemClass itemClass, QUuid handle, QString name) :
    m_type(itemType), m_class(itemClass), m_handle(handle), m_name(name)
{
    Theme *theme = Theme::instance();
    m_icon = theme->icons()->getIcon("cls_novel", ThemeColor::Red);
}

Node::~Node() {
    qDebug() << "Destructor: Node";
}

// Public Methods
// ==============

void Node::pack(QJsonObject &data) {

    QJsonArray children;

    for (qsizetype i=0; i<m_children.size(); ++i) {
        QJsonObject child;
        m_children.at(i)->pack(child);
        children.append(child);
    }

    if (!m_parent) {
        data["x:items"_L1] = children;
    } else {
        switch (m_type) {
            case ItemType::RootType:   data["m:type"_L1] = "Root"_L1; break;
            case ItemType::FolderType: data["m:type"_L1] = "Folder"_L1; break;
            case ItemType::FileType:   data["m:type"_L1] = "File"_L1; break;
            default: return;
        }
        data["u:name"_L1] = m_name;
        data["m:handle"_L1] = m_handle.toString(QUuid::WithoutBraces);
        data["m:order"_L1] = row();
        data["m:characters"_L1] = m_counts.characters;
        data["m:words"_L1] = m_counts.words;
        data["m:expanded"_L1] = m_expanded;
        if (children.size() > 0) {
            data["x:items"_L1] = children;
        }
    }
}

void Node::unpack(const QJsonObject &data) {

}

// Model Access
// ============

int Node::row() const {
    if (m_parent) {
        return m_parent->m_children.indexOf(const_cast<Node*>(this));
    } else {
        return 0;
    }
}

Node *Node::child(int row) {
    if (row < 0 || row >= m_children.size()) {
        return nullptr;
    } else {
        return m_children.at(row);
    }
}

QVariant Node::data(int column, int role) const {
    if (column == 0) {
        if (role == Qt::DisplayRole) return QVariant::fromValue(m_name);
        if (role == Qt::DecorationRole) return QVariant::fromValue(m_icon);
    } else if (column == 1) {
        if (role == Qt::DisplayRole) return QVariant::fromValue(m_counts.words);
        if (role == Qt::TextAlignmentRole) return QVariant::fromValue(Qt::AlignRight);
    }
    return QVariant();
}

// Model Edit
// ==========

void Node::addChild(Node *child, qsizetype pos) {
    child->m_parent = this;
    if (pos >= 0 && pos < m_children.size()) {
        m_children.insert(pos, child);
    } else {
        m_children.append(child);
    }
}

} // namespace Collett
