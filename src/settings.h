/*
** Collett – Main Settings Class
** =============================
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

#ifndef COLLETT_SETTINGS_H
#define COLLETT_SETTINGS_H

#include "collett.h"

#include <QList>
#include <QSize>
#include <QObject>
#include <QScopedPointer>
#include <QTextCharFormat>
#include <QTextBlockFormat>

namespace Collett {

class CollettSettings : public QObject
{
    Q_OBJECT

public:
    static CollettSettings *instance();
    static void destroy();

    explicit CollettSettings();
    ~CollettSettings();

    void flushSettings();

    // Setters
    void setMainWindowSize(const QSize size);
    void setMainSplitSizes(const QList<int> &sizes);

    // Getters
    QSize      mainWindowSize() const;
    QList<int> mainSplitSizes() const;

private:
    static CollettSettings *staticInstance;

    QSize      m_mainWindowSize;
    QList<int> m_mainSplitSizes;

};
} // namespace Collett

#endif // COLLETT_SETTINGS_H
