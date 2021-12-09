/*
** Collett – Project Story Item Class
** ==================================
** 
** This file is a part of Collett
** Copyright 2020–2021, Veronica Berglyd Olsen
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

#ifndef COLLETT_STORYITEM_H
#define COLLETT_STORYITEM_H

#include <QUuid>
#include <QVector>
#include <QVariant>
#include <QJsonObject>

namespace Collett {

class StoryItem : public QObject
{
    Q_OBJECT

public:
    enum ItemType{Root, Book, Partition, Chapter, Scene, Page};

    explicit StoryItem(const QString &name, ItemType type, StoryItem *parentItem=nullptr);
    ~StoryItem();

    // Class Methods

    StoryItem *addChild(const QString &name, ItemType type, int pos=-1);
    QJsonObject toJsonObject();
    bool allowedChild(ItemType type) const;
    bool allowedSibling(ItemType type) const;

    // Class Setters

    void setName(const QString &name);
    void setWordCount(int count);

    // Class Getters

    ItemType type() const;
    QString name() const;
    int wordCount() const;
    int childWordCounts() const;

    // Static Methods

    static QString typeToString(ItemType type);

    // Model Access

    int row() const;
    int childCount() const;
    QVariant data() const;
    StoryItem *child(int row);
    StoryItem *parentItem();

private:
    QVector<StoryItem*> m_childItems;
    StoryItem          *m_parentItem;

    // Member Values

    QUuid    m_handle;
    QString  m_name;
    ItemType m_type;
    int      m_wCount;

};
} // namespace Collett

#endif // COLLETT_STORYITEM_H
