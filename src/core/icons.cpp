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

#include "icons.h"

#include <QBuffer>
#include <QDebug>
#include <QFile>
#include <QImageReader>
#include <QTextStream>

namespace Collett {

// Constructor/Destructor
// ======================

Icons::Icons(const QString &theme) : QQuickImageProvider(QQuickImageProvider::Image)
{
    loadIcons(theme);
}

// Public Methods
// ==============

/**! @brief Render an icon at the requested size, or at its own size.
 */
QImage Icons::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QByteArray svg = m_svg.value(id);
    if (svg.isEmpty()) {
        qWarning() << "Unknown icon:" << id;
        return QImage();
    }

    QBuffer buffer(&svg);
    QImageReader reader(&buffer, "svg");
    if (size) {
        *size = reader.size();
    }
    if (requestedSize.width() > 0 && requestedSize.height() > 0) {
        reader.setScaledSize(requestedSize);
    }
    return reader.read();
}

// Private Methods
// ===============

/**! @brief Load an icon theme file from the application resources.
 *
 * The file has one "key = value" entry per line: "meta:" entries for the
 * theme's details, and "icon:" entries with an SVG each.
 */
bool Icons::loadIcons(const QString &theme)
{
    QFile file(QString(":/qt/qml/Collett/assets/icons/%1.icons").arg(theme));
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Could not open icon theme:" << file.fileName();
        return false;
    }

    QTextStream input(&file);
    while (!input.atEnd()) {
        const QString line = input.readLine();
        const qsizetype eqPos = line.indexOf('=');
        if (eqPos < 0) {
            continue;
        }
        const QString key = line.first(eqPos).trimmed();
        const QString value = line.sliced(eqPos + 1).trimmed();
        if (key.startsWith("icon:")) {
            if (value.startsWith("<svg")) m_svg.insert(key.sliced(5), value.toUtf8());
        } else if (key == "meta:name") {
            m_name = value;
        } else if (key == "meta:author") {
            m_author = value;
        } else if (key == "meta:license") {
            m_license = value;
        }
    }
    qInfo() << "Loaded icon theme:" << m_name << "with" << m_svg.size() << "icons";
    return true;
}

} // namespace Collett
