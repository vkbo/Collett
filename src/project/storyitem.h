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

#ifndef COLLETT_STORYITEM_H
#define COLLETT_STORYITEM_H

#include <QUuid>
#include <QVector>
#include <QVariant>
#include <QJsonObject>

namespace Collett {

class StoryItem
{
public:
    explicit StoryItem(const QString &label, StoryItem *parentItem=nullptr);
    ~StoryItem();

    // Structure
    void appendChild(StoryItem *child);
    QJsonObject toJsonObject();

    // Setters
    void setLabel(const QString &label);
    void setWordCount(int count);

    // Getters
    int wordCount();
    int childWordCounts();

    // Model Access
    int row() const;
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    StoryItem *child(int row);
    StoryItem *parentItem();

private:
    QVector<StoryItem*> m_childItems;
    StoryItem          *m_parentItem;

    // Values
    QUuid   m_handle;
    QString m_label;
    int     m_wCount;

};
} // namespace Collett

#endif // COLLETT_STORYITEM_H
