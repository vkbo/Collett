/*
** Collett - Document Class
** ========================
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
#include "counting.h"

#include <QJsonArray>
#include <QList>
#include <QObject>
#include <QString>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextDocument>

namespace Collett {

/**! @brief The text of all the documents of a group, in reading order.
 *
 * Each document starts with a title block, which holds the title text and,
 * as block properties, the document's handle, level and break settings. The
 * blocks after it, up to the next title, are its text. The document is the
 * source of the group's structure, so every change to it, including those
 * to the structure, can be undone.
 */
class Document : public QTextDocument
{
    Q_OBJECT

public:
    // The block properties of a title block
    enum Property
    {
        HandleProperty = QTextFormat::UserProperty + 1,
        LevelProperty,
        HardBreakProperty,
        NumberedProperty,
    };

    // The values of a document held by its title block
    struct Item
    {
        QString handle;
        QString title;
        ItemLevel level = ItemLevel::SceneLevel;
        bool hardBreak = false;
        bool numbered = true;
    };

    explicit Document(QObject *parent = nullptr);
    ~Document();

    // Load and Save
    void appendItem(const Item &item, const QJsonArray &content);
    QJsonArray packContent(const QString &handle) const;

    // Structure
    QList<Item> items() const;
    QTextBlock titleBlock(const QString &handle) const;
    QString handleAt(int position) const;
    int itemEnd(const QString &handle) const;
    QString itemText(const QString &handle) const;
    CountBlockList snapshotItem(const QString &handle) const;

    // Edits
    int insertItem(int position, const Item &item);
    bool removeItem(const QString &handle);
    int mergeItem(const QString &handle);
    bool setItemTitle(const QString &handle, const QString &title);
    bool setItemValues(const Item &item);
    bool moveItems(int row, int count, int before);
    bool normalize(const QString &newHandle);
    void setHeadingLevel(int first, int last, int level);

    // Static Methods
    static bool isTitle(const QTextBlock &block);
    static Item itemOf(const QTextBlock &block);
    static QTextBlockFormat titleBlockFormat(const Item &item);
    static QTextCharFormat titleCharFormat(ItemLevel level);
    static qreal dividerHeight(ItemLevel level, bool hardBreak);
    static qreal labelHeight();

public slots:
    void refreshTextFormat();

private:
    void appendContent(QTextCursor &cursor, const QJsonArray &content) const;
    QJsonArray packBlocks(QTextBlock block) const;
    void setTitleFormat(QTextCursor &cursor, const Item &item) const;
};
} // namespace Collett
