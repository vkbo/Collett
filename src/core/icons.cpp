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

#include "collett.h"
#include "icons.h"
#include "settings.h"
#include "theme.h"

#include <QString>
#include <QFileInfo>
#include <QTextStream>

namespace Collett {

// Constructor/Destructor
// ======================

Icons::Icons(Theme *parent) : QObject(parent), m_theme(parent) {
    m_settings = Settings::instance();
}

Icons::~Icons() {
    qDebug() << "Destructor: Icons";
}

// Public Methods
// ==============

bool Icons::loadIcons(QString icons) {

    QFileInfo iconsFile = QFileInfo(Settings::assetPath("icons").filePath(icons + ".icons"));
    if (!iconsFile.exists()) return false;
    qInfo() << "Loading Icons:" << icons;

    QFile file(iconsFile.filePath());
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Could not open file:" << iconsFile.filePath();
        return false;
    }

    QTextStream input(&file);
    while (!input.atEnd()) {
        QString line = input.readLine();
        qsizetype eqPos = line.indexOf("=");
        if (eqPos > 5) {
            if (line.startsWith("icon:")) {
                QString key(line.first(eqPos).sliced(5).trimmed());
                QString svg(line.sliced(eqPos + 1).trimmed());
                if (svg.startsWith("<svg")) m_icons[key] = svg;
            } else if (line.startsWith("meta:name")) {
                m_name = line.sliced(eqPos + 1).trimmed();
                qDebug() << "IconSet Name:" << m_name;
            } else if (line.startsWith("meta:author")) {
                m_author = line.sliced(eqPos + 1).trimmed();
                qDebug() << "IconSet Author:" << m_author;
            } else if (line.startsWith("meta:license")) {
                m_license = line.sliced(eqPos + 1).trimmed();
                qDebug() << "IconSet License:" << m_license;
            }
        }
    }
    file.close();

    return true;
}

} // namespace Collett
