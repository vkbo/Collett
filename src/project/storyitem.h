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

class CollettStoryItem
{
public:
    explicit CollettStoryItem(const QVector<QVariant> &data, CollettStoryItem *parentItem=nullptr);
    ~CollettStoryItem();

    void appendChild(CollettStoryItem *child);

    CollettStoryItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    CollettStoryItem *parentItem();

private:
    QVector<CollettStoryItem*> m_childItems;
    QVector<QVariant> m_itemData;
    CollettStoryItem *m_parentItem;

};
} // namespace Collett

#endif // COLLETTSTORYITEM_H
