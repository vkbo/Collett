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

#ifndef COLLETT_NODE_H
#define COLLETT_NODE_H

#include "collett.h"

#include <QIcon>
#include <QJsonObject>
#include <QString>
#include <QUuid>
#include <QVariant>
#include <QVector>

namespace Collett {

class Node : public QObject
{
    Q_OBJECT

    struct Counts {
        qint32 characters;
        qint32 words;
        qint32 paragraphs;
    };

public:
    Node(ItemType itemType, ItemClass itemClass, QUuid handle, QString name);
    Node(ItemType itemType, ItemClass itemClass, QString name) :
        Node(itemType, itemClass, QUuid::createUuid(), name) {};
    ~Node();

    // Methods
    void pack(QJsonObject &data);
    void unpack(const QJsonObject &data);

    // Getters
    ItemType type() const {return m_type;};
    QUuid    handle() const {return m_handle;};
    QString  name() const {return m_name;};
    Counts   counts() {return m_counts;};

    // Model Access
    int row() const;
    int childCount() const {return m_children.count();};
    QVariant data(int column, int role) const;
    Node *child(int row);
    Node *parent() {return m_parent;};

    // Model Edit
    void addChild(Node *child, qsizetype pos = -1);

private:
    // Attributes
    ItemType  m_type;
    ItemClass m_class;
    QUuid     m_handle;
    QString   m_name;
    QIcon     m_icon;

    // Meta
    Counts m_counts = {0, 0, 0};
    bool   m_expanded = false;

    // Structure
    Node           *m_parent = nullptr;
    QVector<Node*>  m_children;

};
} // namespace Collett

#endif // COLLETT_NODE_H
