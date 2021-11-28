/*
Collett – Project Story Item Class
==================================

This file is a part of Collett
Copyright 2020–2021, Veronica Berglyd Olsen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef COLLETTSTORYITEM_H
#define COLLETTSTORYITEM_H

#include <QVector>
#include <QVariant>

namespace Collett {

class StoryItem
{
public:
    explicit StoryItem(const QVector<QVariant> &data, StoryItem *parentItem=nullptr);
    ~StoryItem();

    void appendChild(StoryItem *child);

    StoryItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    StoryItem *parentItem();

private:
    QVector<StoryItem*> m_childItems;
    QVector<QVariant> m_itemData;
    StoryItem *m_parentItem;

};
} // namespace Collett

#endif // COLLETTSTORYITEM_H
