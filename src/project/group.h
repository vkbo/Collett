/*
** Collett - Project Group Class
** =============================
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
#include "node.h"

#include <QCoreApplication>
#include <QJsonObject>
#include <QList>
#include <QString>

namespace Collett {

/**! @brief A top level group of project documents, like the novel.
 *
 * The group owns its documents, which are kept in reading order. The
 * documents and their text are stored together in one file, which is named
 * in the project structure.
 */
class Group
{
    Q_DECLARE_TR_FUNCTIONS(Group)

public:
    Group(const QString &name, ItemClass itemClass);
    ~Group();

    // Methods
    void pack(QJsonObject &data, int order) const;
    static Group *unpack(const QJsonObject &data);

    // Getters
    QString name() const { return m_name; };
    QString fileName() const { return m_fileName; };
    ItemClass itemClass() const { return m_class; };
    bool isModified() const { return m_modified; };
    const QList<Node *> &items() const { return m_items; };
    qsizetype count() const { return m_items.size(); };
    Node *item(qsizetype pos) const { return m_items.value(pos, nullptr); };

    // Setters
    void setName(const QString &name) { m_name = name.simplified(); };
    void setFileName(const QString &fileName) { m_fileName = fileName; };
    void setModified(bool state) { m_modified = state; };

    // Edit
    void appendItem(Node *node) { m_items.append(node); };
    void insertItem(qsizetype pos, Node *node) { m_items.insert(qBound(0, pos, m_items.size()), node); };
    Node *takeItem(qsizetype pos) { return (pos >= 0 && pos < m_items.size()) ? m_items.takeAt(pos) : nullptr; };

    // Static Methods
    static bool classFromString(const QString &value, ItemClass &itemClass);
    static QString classToString(ItemClass itemClass);
    static bool isFileName(const QString &value);

private:
    QString m_name;
    QString m_fileName;
    ItemClass m_class;

    // A new group has not been saved yet, so it starts out modified
    bool m_modified = true;

    QList<Node *> m_items;
};
} // namespace Collett
