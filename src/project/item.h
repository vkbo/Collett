/*
** Collett – Project Item Class
** ============================
** 
** This file is a part of Collett
** Copyright 2020–2022, Veronica Berglyd Olsen
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

#ifndef COLLETT_ITEM_H
#define COLLETT_ITEM_H

#include <QUuid>
#include <QVector>
#include <QVariant>
#include <QJsonObject>

namespace Collett {

class Item : public QObject
{
    Q_OBJECT

public:
    enum ItemType{
        Invalid,
        Root, Book, Partition, Chapter, Scene, Page,
        Group, Note
    };

    explicit Item(const QUuid &uuid, const QString &name, bool story, ItemType type, Item *parentItem=nullptr);
    ~Item();

    // Class Methods

    Item *addChild(const QString &name, ItemType type, int pos=-1);
    Item *addChild(const QJsonObject &json);
    QJsonObject toJsonObject();
    bool allowedChild(ItemType type) const;
    bool allowedSibling(ItemType type) const;
    bool canHoldDocument() const;

    // Class Setters

    void setName(const QString &name);
    void setWordCount(int count);
    void setExpanded(bool state);

    // Class Getters

    ItemType type() const;
    QUuid handle() const;
    QString name() const;
    int wordCount() const;
    int childWordCounts() const;
    bool isExpanded() const;
    Item *findItemFromHandle(const QUuid &uuid) const;

    // Static Methods

    static QString typeToLabel(ItemType type);
    static ItemType typeFromString(const QString &value);

    // Model Access

    int row() const;
    int childCount() const;
    QVariant data() const;
    Item *child(int row);
    Item *parentItem();

private:
    QVector<Item*> m_childItems;
    Item          *m_parentItem;

    // Member Values

    bool      m_story;
    QUuid     m_handle;
    QString   m_name;
    ItemType  m_type;
    int       m_words;
    bool      m_expanded;

};
} // namespace Collett

#endif // COLLETT_ITEM_H
