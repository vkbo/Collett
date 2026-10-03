/*
** Collett - Common Labels
** =======================
**
** This file is a part of Collett
** Copyright (C) 2026 Veronica Berglyd Olsen
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

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

namespace Collett {

/**! @brief Translated names of terms used across the app.
 *
 * Terms that appear in many places are translated once, here, so they read
 * the same everywhere. Names of things in the project are in the "Label"
 * context. Text statistics are in the "Stats" context, which keeps text
 * characters apart from the characters of a story.
 */
class Labels : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    enum Stat
    {
        Words,
        Characters,
        Paragraphs,
    };
    Q_ENUM(Stat)

    explicit Labels(QObject *parent = nullptr) : QObject(parent) {};

    Q_INVOKABLE static QString levelName(int level);
    Q_INVOKABLE static QString className(int itemClass);
    Q_INVOKABLE static QString statName(Stat stat);
    Q_INVOKABLE static QString wordCount(int words);
    Q_INVOKABLE static QString levelNumber(int level, bool numbered, int number, int chapterNumber);
    Q_INVOKABLE static QString itemName(const QString &title, int level, bool numbered, int number, int chapterNumber);
};

} // namespace Collett
