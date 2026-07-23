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
#include "projectmodel.h"

#include <QJsonObject>
#include <QRandomGenerator>
#include <QString>

using namespace Qt::Literals::StringLiterals;

namespace Collett {

// Constructor/Destructor
// ======================

Tree::Tree(QObject *parent) : QObject(parent)
{
    m_model = new ProjectModel(this);
}

Tree::~Tree()
{
    qDebug() << "Destructor: Tree";
}

// Public Methods
// ==============

void Tree::pack(QJsonObject &data)
{
    data["c:format"_L1] = "CollettProjectStructure";
    if (m_model) m_model->pack(data);
}

void Tree::unpack(const QJsonObject &data)
{
    if (m_model) {
        qDebug() << "Unpacking project tree";
        m_model->unpack(data);
    }
}

// Data Methods
// ============

/**!
 * @brief Add a node to the nodes map.
 *
 * @param node The node to be added to the map.
 */
void Tree::addNode(Node *node)
{
    if (node) m_nodes.insert(node->handle(), node);
}

/**!
 * @brief Remove a node from the nodes map.
 *
 * @param handle The handle of the node to remove.
 */
void Tree::removeNode(const QString &handle)
{
    if (m_nodes.contains(handle)) m_nodes.remove(handle);
}

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

} // namespace Collett
