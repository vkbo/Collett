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

#pragma once

#include "collett.h"
#include "group.h"
#include "node.h"
#include "projectmodel.h"

#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QString>

namespace Collett {

class Tree : public QObject
{
    Q_OBJECT

public:
    explicit Tree(QObject *parent = nullptr);
    ~Tree();

    // Getters
    ProjectModel *model() { return m_model; };
    const QList<Group *> &groups() const { return m_groups; };
    Node *node(const QString &handle) { return m_nodes.value(handle, nullptr); };
    bool isModified() const { return m_modified; };

    // Setters
    void setModified(bool state) { m_modified = state; };

    // Methods
    void pack(QJsonObject &data);
    void unpack(const QJsonObject &data);
    void showNovelGroup();

    // Data Methods
    QString newHandle() const;
    Node *createNode(const QString &handle, ItemLevel level);
    bool addNode(Group *group, Node *node);
    void forgetNode(const QString &handle) { m_nodes.remove(handle); };

    // Static Methods
    static bool isHandle(const QString &value);

private:
    // The list of groups has not been saved yet, so it starts out modified
    bool m_modified = true;
    QString m_createdTime;

    ProjectModel *m_model;
    QList<Group *> m_groups;
    QHash<QString, Node *> m_nodes;

    void clear();
    void assignContentName(Group *group);
};
} // namespace Collett
