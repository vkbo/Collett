/*
** Collett – Icons Class
** =====================
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

#ifndef COLLETT_ICONS_H
#define COLLETT_ICONS_H

#include "collett.h"
#include "settings.h"

#include <QByteArray>
#include <QIcon>
#include <QMap>
#include <QSize>
#include <QString>

namespace Collett {

class Theme;
class Icons : public QObject
{
    Q_OBJECT

public:
    explicit Icons(Theme *parent);
    ~Icons();

    // Getters
    QIcon getIcon(QString name, ThemeColor color, QSize size);
    QIcon getIcon(QString name, ThemeColor color) {return getIcon(name, color, QSize(24, 24));};
    QIcon getProjectIcon(ItemType itemType, ItemClass itemClass, ItemLevel itemLevel, QSize size);

    // Methods
    bool loadIcons(QString icons);

private:
    Theme    *m_theme;
    Settings *m_settings;

    // Meta
    QString m_name = "";
    QString m_author = "";
    QString m_license = "";

    // Storage
    QMap<QString, QByteArray> m_svg;
    QMap<QString, QIcon>      m_icons;

    // Functions
    QIcon generateIcon(QString name, ThemeColor color, QSize size);
};
} // namespace Collett

#endif // COLLETT_ICONS_H
