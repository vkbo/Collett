/*
** Collett - Icons Class
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

#pragma once

#include <QByteArray>
#include <QHash>
#include <QImage>
#include <QQuickImageProvider>
#include <QSize>
#include <QString>

namespace Collett {

/**! @brief Provides the icons of an icon theme file to QML.
 *
 * Icons are loaded as "image://icons/<key>". They are drawn in black, and
 * are meant to be coloured by the item showing them, like the icon colour
 * of a control.
 */
class Icons : public QQuickImageProvider
{
public:
    explicit Icons(const QString &theme);

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    // Getters
    QString name() const { return m_name; };
    QString author() const { return m_author; };
    QString license() const { return m_license; };

private:
    // Meta
    QString m_name;
    QString m_author;
    QString m_license;

    // Storage
    QHash<QString, QByteArray> m_svg;

    bool loadIcons(const QString &theme);
};

} // namespace Collett
