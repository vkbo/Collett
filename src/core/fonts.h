/*
** Collett - Font Tools
** ====================
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

#include <QFont>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QtQml/qqmlregistration.h>

namespace Collett {

/**! @brief Font lookups for QML, which has no access to the font database.
 */
class Fonts : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit Fonts(QObject *parent = nullptr) : QObject(parent) {};

    Q_INVOKABLE static QStringList families(bool fixedPitch);
    Q_INVOKABLE static QStringList styles(const QString &family);
    Q_INVOKABLE static QString styleOf(const QFont &font);
    Q_INVOKABLE static QList<int> sizes();
    Q_INVOKABLE static QFont font(const QString &family, const QString &style, qreal pointSize);
    Q_INVOKABLE static QFont scaled(const QFont &font, qreal factor);
    Q_INVOKABLE static QString describe(const QFont &font);
    Q_INVOKABLE static QFont interfaceFont(const QFont &font);
};

} // namespace Collett
