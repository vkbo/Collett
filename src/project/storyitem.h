/*
Collett – Project Item Class
============================

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

#ifndef COL_STORYITEM_H
#define COL_STORYITEM_H

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QXmlStreamWriter>
#include <QDomNode>

namespace Collett {

class StoryItem : public QObject
{
    Q_OBJECT

public:
    enum ItemType {Partition, Chapter, Section, Scene, Page};

    StoryItem();
    ~StoryItem() {};

    void populateItem(ItemType type, const QString &title, const QString &handle);
    void initItem(ItemType type, const QString &title);

    void toXml(const QString &nsCol, const QString &nsItm, QXmlStreamWriter &xmlWriter);
    void fromXml(const QString &nsCol, const QString &nsItm, QDomNode &node);

    QJsonObject toJson();

    bool isEmpty() {
        return m_empty;
    }

    // Getters
    QString  title() const;
    QString  handle() const;
    ItemType type() const;
    QString  typeAsString() const;
    QString  typeForDisplay() const;
    QString  typeBackground() const;

    int cursorPosition() const;
    int itemCount() const;
    int wordCount() const;

    // Setters
    void setTitle(const QString &title);
    void setHandle(const QString &handle);
    void setType(ItemType type);

    void setCursorPosition(int position);
    void setItemCount(int count);
    void setWordCount(int count);

private:

    bool m_empty;

    QString  m_title;
    QString  m_handle;
    ItemType m_type;

    int m_itemCount;
    int m_cursorPos;
    int m_wordCount;

};
} // namespace Collett

#endif // COL_STORYITEM_H
