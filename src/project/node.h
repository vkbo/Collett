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
#include <QList>
#include <QString>
#include <QUuid>
#include <QVariant>

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
    Node(ItemType itemType, QUuid handle, QString name);
    ~Node();

    // Methods
    void pack(QJsonObject &data);
    void unpack(const QJsonObject &data, int &skipped, int &errors);

    // Getters
    ItemType  itemType() const {return m_type;};
    ItemClass itemClass() const {return m_class;};
    ItemLevel itemLevel() const {return m_level;};
    QUuid     handle() const {return m_handle;};
    QString   name() const {return m_name;};
    Counts    counts() {return m_counts;};
    bool      isExpanded() {return m_expanded;};

    // Setters
    void setName(QString name) {m_name = name.simplified();};
    void setCounts(Counts counts) {m_counts = counts;};
    void setExpanded(bool state) {m_expanded = state;};
    void setActive(bool state);

    // Model Access
    int row() const;
    int childCount() const {return m_children.count();};
    QVariant data(int column, int role) const;
    Qt::ItemFlags flags() const {return m_flags;};
    Node *child(int row);
    Node *parent() {return m_parent;};

    QList<Node*> allChildren();

    // Model Edit
    bool canAddRoot();
    bool canAddFolder();
    bool canAddFile(ItemLevel itemLevel);

    void  addChild(Node *child, qsizetype pos = -1);
    Node *createRoot(QUuid handle, QString name, ItemClass itemClass);
    Node *createFolder(QUuid handle, QString name);
    Node *createFile(QUuid handle, QString name, ItemLevel itemLevel);
    void  updateIcon();

    // Static Methods
    static bool typeFromString(QString value, ItemType &itemType);
    static bool classFromString(QString value, ItemClass &itemClass);
    static bool levelFromString(QString value, ItemLevel &itemLevel);

private:
    // Attributes
    ItemType      m_type;
    ItemClass     m_class;
    ItemLevel     m_level;
    QUuid         m_handle;
    QString       m_name;
    bool          m_active = false;
    Qt::ItemFlags m_flags = Qt::NoItemFlags;

    // Meta
    QIcon  m_icon;
    Counts m_counts = {0, 0, 0};
    bool   m_expanded = false;
    QIcon  m_activeIcon;

    // Accessibility
    QString m_accWords = tr("Word Count: %1");
    QString m_accActive = "";

    // Structure
    Node         *m_parent = nullptr;
    QList<Node*>  m_children;

    // Methods
    void recursiveAppendChildren(QList<Node*> &children);
    void updateValues();
};
} // namespace Collett

#endif // COLLETT_NODE_H
