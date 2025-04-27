/*
** Collett – Project Tree Class
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

#ifndef COLLETT_TREE_H
#define COLLETT_TREE_H

#include "collett.h"
#include "node.h"
#include "projectmodel.h"

#include <QHash>
#include <QJsonObject>
#include <QUuid>

namespace Collett {

class Tree : public QObject
{
    Q_OBJECT

public:
    explicit Tree(QObject *parent = nullptr);
    ~Tree();

    // Getters
    ProjectModel *model() {return m_model;};
    Node *node(const QUuid &uuid) {return m_nodes.value(uuid, nullptr);};

    // Methods
    void pack(QJsonObject &data);
    void unpack(const QJsonObject &data);

    // Data Methods
    void addNode(Node *node);
    void removeNode(const QUuid &uuid);

private:
    ProjectModel *m_model;
    QHash<QUuid, Node*> m_nodes;

};
} // namespace Collett

#endif // COLLETT_TREE_H
