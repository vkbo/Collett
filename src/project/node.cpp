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
#include "tools.h"
#include "tree.h"

#include <QIcon>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QUuid>
#include <QVariant>

using namespace Qt::Literals::StringLiterals;

namespace Collett {

// Constructor/Destructor
// ======================

Node::Node(Tree *tree, ItemType itemType, QUuid handle, QString name) :
    m_type(itemType), m_handle(handle), m_name(name), m_tree(tree)
{
    m_class = ItemClass::NovelClass;
    m_level = ItemLevel::PageLevel;
    m_icon = QIcon();
    m_activeIcon = QIcon();
    if (itemType == ItemType::RootType) {
        m_flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled;
    } else if (itemType == ItemType::FileType || itemType == ItemType::FolderType) {
        m_flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled;
    }
}

Node::~Node() {
    qDebug() << "Destructor: Node" << m_name;
    qDeleteAll(m_children);  // Parent is tracked in m_parent, so we delete explicitly
}

// Setters
// =======

void Node::setActive(bool state) {
    Theme *theme = Theme::instance();
    if (m_type == ItemType::InvisibleRoot) {
        m_icon = QIcon();
    } else {
        m_active = state;
        if (m_type == ItemType::FileType) {
            if (m_active) {
                m_accActive = tr("Active");
                m_activeIcon = theme->icons()->getIcon("checked", ThemeColor::Green, theme->baseIconSize());
            } else {
                m_accActive = tr("Inactive");
                m_activeIcon = theme->icons()->getIcon("unchecked", ThemeColor::Red, theme->baseIconSize());
            }
        } else {
            m_accActive = "";
            m_activeIcon = theme->icons()->getIcon("noncheckable", ThemeColor::FadedColor, theme->baseIconSize());
        }
    }
}

// Checkers
// ========

/**!
 * @brief Return true if the node is allowed to be a novel document.
 *
 * Novel documents are only allowed under the Novel root folder, and in
 * the Archive or Trash folder.
 *
 * @return Returns true if the node is of Novel, Archive or Trash class.
 */
bool Node::isDocumentAllowed() {
    switch (m_class) {
        case ItemClass::NovelClass:
        case ItemClass::ArchiveClass:
        case ItemClass::TrashClass:
            return true;
        case ItemClass::CharacterClass:
        case ItemClass::PlotClass:
        case ItemClass::LocationClass:
        case ItemClass::ObjectClass:
        case ItemClass::EntityClass:
        case ItemClass::CustomClass:
            return false;
    }
    return false;
}

/**!
 * @brief Return true if the node is allowed to be a project note.
 *
 * Notes are allowed to exist anywhere in the project except under Novel root
 * folders.
 *
 * @return Returns true if the node is allowed to be a project note.
 */
bool Node::isNoteAllowed() {
    return m_class != ItemClass::NovelClass;
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
        if (m_type == ItemType::RootType) {
            switch (m_class) {
                case ItemClass::NovelClass:     data["m:class"_L1] = "Novel"_L1; break;
                case ItemClass::CharacterClass: data["m:class"_L1] = "Character"_L1; break;
                case ItemClass::PlotClass:      data["m:class"_L1] = "Plot"_L1; break;
                case ItemClass::LocationClass:  data["m:class"_L1] = "Location"_L1; break;
                case ItemClass::ObjectClass:    data["m:class"_L1] = "Object"_L1; break;
                case ItemClass::EntityClass:    data["m:class"_L1] = "Entity"_L1; break;
                case ItemClass::CustomClass:    data["m:class"_L1] = "Custom"_L1; break;
                case ItemClass::ArchiveClass:   data["m:class"_L1] = "Archive"_L1; break;
                case ItemClass::TrashClass:     data["m:class"_L1] = "Trash"_L1; break;
                default: return;
            }
        }
        if (m_type == ItemType::FileType) {
            switch (m_level) {
                case ItemLevel::PageLevel:    data["m:level"_L1] = "Page"_L1; break;
                case ItemLevel::NoteLevel:    data["m:level"_L1] = "Note"_L1; break;
                case ItemLevel::TitleLevel:   data["m:level"_L1] = "Title"_L1; break;
                case ItemLevel::ChapterLevel: data["m:level"_L1] = "Chapter"_L1; break;
                case ItemLevel::SceneLevel:   data["m:level"_L1] = "Scene"_L1; break;
                default: return;
            }
            data["u:active"_L1] = m_active;
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

void Node::unpack(const QJsonObject &data, int &skipped, int &errors) {

    if (data.isEmpty()) {
        qWarning() << "Received a project node with no data";
        skipped++;
        errors++;
        return;
    }

    bool error = false;

    QString   name      = "";
    QUuid     handle    = QUuid();
    ItemType  itemType  = ItemType::FileType;
    ItemClass itemClass = ItemClass::NovelClass;
    ItemLevel itemLevel = ItemLevel::PageLevel;
    Counts    counts    = {0, 0, 0};
    bool      expanded  = false;
    bool      active    = false;

    // Name (Optional)
    if (data.contains("u:name"_L1)) {
        name = data["u:name"_L1].toString();
    }
    if (name.isEmpty()) {
        name = tr("Unnamed");
    }

    // Handle (Required)
    if (data.contains("m:handle"_L1)) {
        handle = QUuid(data["m:handle"_L1].toString());
    }
    if (handle.isNull()) {
        qWarning() << "Received a project node with invalid handle";
        error &= true;
        errors++;
    }

    // Item Type (Required)
    if (!Node::typeFromString(JsonUtils::getJsonString(data, "m:type"_L1, "Error"), itemType)) {
        qWarning() << "Received a project node with invalid type";
        error &= true;
        errors++;
    }

    // Other Values
    if (data.contains("u:active"_L1)) {
        active = data["u:active"_L1].toBool();
    }

    // Meta Values
    if (data.contains("m:words"_L1)) {
        counts.words = data["m:words"_L1].toInt();
    }
    if (data.contains("m:characters"_L1)) {
        counts.characters = data["m:characters"_L1].toInt();
    }
    if (data.contains("m:expanded"_L1)) {
        expanded = data["m:expanded"_L1].toBool();
    }

    // Error Handling
    if (error) {
        qWarning() << "Skipping project node with name " << name << "due to errors";
        skipped++;
        return;
    }

    Node *node = nullptr;
    switch (itemType) {
        case ItemType::RootType:
            if (!Node::classFromString(JsonUtils::getJsonString(data, "m:class"_L1, "Error"), itemClass)) {
                qWarning() << "Received a project root node with invalid class";
                errors++;
            }
            node = this->createRoot(handle, name, itemClass);
            this->addChild(node);
            break;
        case ItemType::FolderType:
            node = this->createFolder(handle, name);
            this->addChild(node);
            break;
        case ItemType::FileType:
            if (!Node::levelFromString(JsonUtils::getJsonString(data, "m:level"_L1, "Error"), itemLevel)) {
                qWarning() << "Received a project node with invalid level";
                errors++;
            }
            node = this->createFile(handle, name, itemLevel);
            this->addChild(node);
            break;
        default:
            break;
    }

    if (node) {
        qDebug() << "Added node with handle" << handle.toString(QUuid::WithoutBraces);
    } else {
        qWarning() << "Failed to add node with handle" << handle.toString(QUuid::WithoutBraces);
        skipped++;
        return;
    }

    node->setCounts(counts);
    node->setExpanded(expanded);
    node->setActive(active);

    if (data.contains("x:items"_L1)) {
        if (data["x:items"_L1].isArray()) {
            for (const QJsonValue &value : data["x:items"_L1].toArray()) {
                if (value.isObject()) {
                    node->unpack(value.toObject(), skipped, errors);
                } else {
                    qWarning() << "Item: Child item is not a JSON object";
                }
            }
        }
    }
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
    switch (column) {
        case 0:
            switch (role) {
                case Qt::DisplayRole:
                case Qt::ToolTipRole:
                case Qt::AccessibleTextRole:
                    return QVariant::fromValue(m_name);
                case Qt::DecorationRole:
                    return QVariant::fromValue(m_icon);
            }
            break;
        case 1:
            switch (role) {
                case Qt::DisplayRole:
                    return QVariant::fromValue(m_counts.words);
                case Qt::ToolTipRole:
                case Qt::AccessibleTextRole:
                    return QVariant::fromValue(m_accWords.arg(m_counts.words));
                case Qt::TextAlignmentRole:
                    return QVariant::fromValue(Qt::AlignRight);
            }
            break;
        case 2:
            switch (role) {
                case Qt::DecorationRole:
                    return QVariant::fromValue(m_activeIcon);
                case Qt::ToolTipRole:
                case Qt::AccessibleTextRole:
                    return QVariant::fromValue(m_accActive);
            }
            break;
    }
    return QVariant();
}

QList<Node*> Node::allChildren() {
    QList<Node*> children;
    this->recursiveAppendChildren(children);
    return children;
}

// Model Edit
// ==========

void Node::addChild(Node *child, qsizetype pos) {
    m_tree->addNode(child);
    child->m_parent = this;
    if (pos >= 0 && pos < m_children.size()) {
        m_children.insert(pos, child);
    } else {
        m_children.append(child);
    }
    child->updateIcon();
    child->updateValues();
}

Node *Node::takeChild(qsizetype pos) {
    if (pos >= 0 && pos < m_children.count()) {
        Node *child = m_children.takeAt(pos);
        m_tree->removeNode(child->handle());
        return child;
    }
    return nullptr;
}

bool Node::canAddRoot() {
    if (m_type == ItemType::InvisibleRoot) {
        return true;
    } else {
        qWarning() << "Root nodes can only be added to invisible root";
        return false;
    }
}

bool Node::canAddFolder() {
    if (m_type != ItemType::InvisibleRoot) {
        return true;
    } else {
        qWarning() << "Folder nodes cannot be added to invisible root";
        return false;
    }
}

bool Node::canAddFile(ItemLevel itemLevel) {
    if (m_type == ItemType::InvisibleRoot) {
        qWarning() << "File nodes cannot be added to invisible root";
        return false;
    }

    switch (m_class) {
        case ItemClass::NovelClass:
        case ItemClass::ArchiveClass:
            return itemLevel != ItemLevel::NoteLevel;

        case ItemClass::CharacterClass:
        case ItemClass::PlotClass:
        case ItemClass::LocationClass:
        case ItemClass::ObjectClass:
        case ItemClass::EntityClass:
        case ItemClass::CustomClass:
            return itemLevel == ItemLevel::NoteLevel;

        case ItemClass::TrashClass:
            return false;
    }

    return false;
}

Node *Node::createRoot(QUuid handle, QString name, ItemClass itemClass) {
    Node *node = new Node(m_tree, ItemType::RootType, handle, name);
    node->m_class = itemClass;
    return node;
}

Node *Node::createFolder(QUuid handle, QString name) {
    Node *node = new Node(m_tree, ItemType::FolderType, handle, name);
    node->m_class = m_class;
    return node;
}

Node *Node::createFile(QUuid handle, QString name, ItemLevel itemLevel) {
    Node *node = new Node(m_tree, ItemType::FileType, handle, name);
    node->m_class = m_class;
    node->m_level = itemLevel;
    return node;
}

void Node::updateIcon() {
    Theme *theme = Theme::instance();
    if (m_type == ItemType::InvisibleRoot) {
        m_icon = QIcon();
    } else {
        m_icon = theme->icons()->getProjectIcon(m_type, m_class, m_level, theme->baseIconSize());
    }
}

void Node::updateValues() {
    if (m_parent && m_parent->itemType() != ItemType::InvisibleRoot) {
        m_class = m_parent->m_class;
        if (this->isFileType()) {
            if (this->isDocument() && !this->isDocumentAllowed()) {
                m_level = ItemLevel::NoteLevel;
                this->updateIcon();
            }
            if (this->isNote() && !this->isNoteAllowed()) {
                m_level = ItemLevel::PageLevel;
                this->updateIcon();
            }
        }
    }
}

// Static Methods
// ==============

bool Node::typeFromString(QString value, ItemType &itemType) {
    if (value == "File") {
        itemType = ItemType::FileType;
        return true;
    }
    if (value == "Folder") {
        itemType = ItemType::FolderType;
        return true;
    }
    if (value == "Root") {
        itemType = ItemType::RootType;
        return true;
    }
    return false;
}

bool Node::classFromString(QString value, ItemClass &itemClass) {
    if (value == "Novel") {
        itemClass = ItemClass::NovelClass;
        return true;
    }
    if (value == "Character") {
        itemClass = ItemClass::CharacterClass;
        return true;
    }
    if (value == "Plot") {
        itemClass = ItemClass::PlotClass;
        return true;
    }
    if (value == "Location") {
        itemClass = ItemClass::LocationClass;
        return true;
    }
    if (value == "Object") {
        itemClass = ItemClass::ObjectClass;
        return true;
    }
    if (value == "Entity") {
        itemClass = ItemClass::EntityClass;
        return true;
    }
    if (value == "Custom") {
        itemClass = ItemClass::CustomClass;
        return true;
    }
    if (value == "Archive") {
        itemClass = ItemClass::ArchiveClass;
        return true;
    }
    if (value == "Trash") {
        itemClass = ItemClass::TrashClass;
        return true;
    }
    return false;
}

bool Node::levelFromString(QString value, ItemLevel &itemLevel) {
    if (value == "Page") {
        itemLevel = ItemLevel::PageLevel;
        return true;
    }
    if (value == "Note") {
        itemLevel = ItemLevel::NoteLevel;
        return true;
    }
    if (value == "Title") {
        itemLevel = ItemLevel::TitleLevel;
        return true;
    }
    if (value == "Chapter") {
        itemLevel = ItemLevel::ChapterLevel;
        return true;
    }
    if (value == "Scene") {
        itemLevel = ItemLevel::SceneLevel;
        return true;
    }
    return false;
}

// Private Methods
// ===============

void Node::recursiveAppendChildren(QList<Node*> &children) {
    for (Node *child : m_children) {
        children.append(child);
        child->recursiveAppendChildren(children);
    }
}

} // namespace Collett
