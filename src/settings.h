/*
Collett – Main Settings Class
=============================

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

#ifndef COLLETTSETTINGS_H
#define COLLETTSETTINGS_H

#include "collett.h"

#include <QList>
#include <QObject>
#include <QScopedPointer>
#include <QSize>

namespace Collett {

class CollettSettingsPrivate;
class CollettSettings : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(CollettSettings)

public:
    static CollettSettings *instance();
    ~CollettSettings();

private:
    QScopedPointer<CollettSettingsPrivate> d_ptr;
    CollettSettings();

public:
    void flushSettings();

    // Setters
    void setMainWindowSize(const QSize size);
    void setMainSplitSizes(const QList<int> &sizes);

    // Getters
    QSize      mainWindowSize() const;
    QList<int> mainSplitSizes() const;
};
} // namespace Collett

#endif // COLLETTSETTINGS_H
