/*
** Collett - Project Tree Class
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

#include "tree.h"
#include "labels.h"
#include "projectmodel.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QPair>
#include <QRandomGenerator>
#include <QSet>
#include <QString>

#include <algorithm>

using namespace Qt::Literals::StringLiterals;

namespace Collett {

// Constructor/Destructor
// ======================

Tree::Tree(QObject *parent) : QObject(parent)
{
    m_model = new ProjectModel(this);
    connect(m_model, &ProjectModel::structureChanged, this, [this]() {
        if (Group *group = m_model->group()) group->setModified(true);
    });
    showNovelGroup();
}

Tree::~Tree()
{
    qDebug() << "Destructor: Tree";
    m_model->setGroup(nullptr);
    qDeleteAll(m_groups);
}

// Public Methods
// ==============

void Tree::pack(QJsonObject &data)
{
    QJsonArray groups;
    for (qsizetype i = 0; i < m_groups.size(); ++i) {
        QJsonObject group;
        m_groups.at(i)->pack(group, i);
        groups.append(group);
    }

    data["c:format"_L1] = "CollettProjectStructure:1.0";
    data["x:groups"_L1] = groups;
}

/**! @brief Read the project structure from a JSON object.
 *
 * Groups are sorted by their order values. Their documents are added from
 * the group files afterwards, with addNode. A group without a valid file
 * name, or with the same file as a group before it, gets a new one.
 */
void Tree::unpack(const QJsonObject &data)
{
    qDebug() << "Unpacking project tree";
    this->clear();

    QList<QPair<int, Group *>> ordered;
    const QJsonArray groups = data["x:groups"_L1].toArray();
    for (qsizetype i = 0; i < groups.size(); ++i) {
        Group *group = Group::unpack(groups.at(i).toObject());
        if (!group) continue;
        ordered.append({groups.at(i)["m:order"_L1].toInt(int(i)), group});
    }
    std::stable_sort(ordered.begin(), ordered.end(), [](const auto &a, const auto &b) { return a.first < b.first; });

    // What was read matches what is on disk, unless a group had to be added
    // or given a file name
    m_modified = false;
    QSet<QString> fileNames;
    for (const auto &entry : ordered) {
        Group *group = entry.second;
        const QString fileName = group->fileName();
        if (fileNames.contains(fileName)) {
            qWarning() << "Duplicate project group file" << fileName;
            group->setFileName(QString());
        } else if (!fileName.isEmpty()) {
            fileNames.insert(fileName);
        }
        m_groups.append(group);
    }
    for (Group *group : std::as_const(m_groups)) {
        assignFileName(group);
    }
    showNovelGroup();
}

/**! @brief Make sure there is a novel group, and show it in the model.
 *
 * Until there is a way to switch between groups, the model shows the first
 * novel group. The model is reset, so it also picks up documents added to
 * the group with addNode.
 */
void Tree::showNovelGroup()
{
    Group *novel = nullptr;
    for (Group *group : std::as_const(m_groups)) {
        if (group->itemClass() == ItemClass::NovelClass) {
            novel = group;
            break;
        }
    }
    if (!novel) {
        novel = new Group(Labels::className(ItemClass::NovelClass), ItemClass::NovelClass);
        m_groups.prepend(novel);
        assignFileName(novel);
        m_modified = true;
    }
    m_model->setGroup(novel);
}

// Data Methods
// ============

/**!
 * @brief Generate a new unique node handle.
 *
 * The handle is a 13 character lowercase hex string generated from a 52-bit
 * random number. In the unlikely event that it collides with a handle already
 * in use, a new one is generated.
 *
 * @return QString The new handle.
 */
QString Tree::newHandle() const
{
    QString handle;
    do {
        quint64 value = QRandomGenerator::global()->generate64() & ((Q_UINT64_C(1) << 52) - 1);
        handle = QString::number(value, 16).rightJustified(13, u'0');
    } while (m_nodes.contains(handle));
    return handle;
}

/**! @brief Create a document node with a new handle, and register it.
 *
 * The node is not added to a group. The caller adds it through the model,
 * which owns the order of the documents.
 */
Node *Tree::createNode(ItemLevel level)
{
    Node *node = new Node(newHandle(), QString(), level);
    m_nodes.insert(node->handle(), node);
    return node;
}

/**! @brief Add a node read from a file at the end of a group, and register
 * it.
 *
 * A node with a handle that is already in use is not added, and the caller
 * still owns it.
 */
bool Tree::addNode(Group *group, Node *node)
{
    if (m_nodes.contains(node->handle())) {
        qWarning() << "Duplicate project item handle" << node->handle();
        return false;
    }
    m_nodes.insert(node->handle(), node);
    group->appendItem(node);
    return true;
}

// Static Methods
// ==============

/**!
 * @brief Check if a string is a valid node handle.
 *
 * @param value The string to check.
 * @return bool True if the string is a 13 character lowercase hex string.
 */
bool Tree::isHandle(const QString &value)
{
    if (value.size() != 13) {
        return false;
    }
    for (const QChar &c : value) {
        if ((c < u'0' || c > u'9') && (c < u'a' || c > u'f')) {
            return false;
        }
    }
    return true;
}

// Private Methods
// ===============

void Tree::clear()
{
    m_model->setGroup(nullptr);
    m_nodes.clear();
    qDeleteAll(m_groups);
    m_groups.clear();
}

/**! @brief Give a group without a file name the first free one, counting
 * from "document1.json".
 */
void Tree::assignFileName(Group *group)
{
    if (!group->fileName().isEmpty()) {
        return;
    }
    QSet<QString> used;
    for (const Group *other : std::as_const(m_groups)) {
        used.insert(other->fileName());
    }
    int number = 1;
    while (used.contains(u"document%1.json"_s.arg(number))) {
        ++number;
    }
    group->setFileName(u"document%1.json"_s.arg(number));
    m_modified = true;
}

} // namespace Collett
