/*
** Collett – Core Icon Repository
** ==============================
**
** This file is a part of Collett
** Copyright 2020–2021, Veronica Berglyd Olsen
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
#include "svgiconengine.h"

#include <QIcon>
#include <QColor>
#include <QDebug>
#include <QString>
#include <QPalette>
#include <QApplication>

namespace Collett {

CollettIcons *CollettIcons::staticInstance = nullptr;
CollettIcons *CollettIcons::instance() {
    if (staticInstance == nullptr) {
        staticInstance = new CollettIcons();
        qDebug() << "Constructor: CollettIcons";
    }
    return staticInstance;
}

void CollettIcons::destroy() {
    if (staticInstance != nullptr) {
        delete CollettIcons::staticInstance;
    }
}

/**!
 * @brief Construct a CollettIcons object.
 * 
 * The icon SVG data is from https://remixicon.com
 */
CollettIcons::CollettIcons() {

    // RemixIcon: archive-fill
    m_svgData["archive"] = QString(
        "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' width='24' height='24'>"
            "<path fill='none' d='M0 0h24v24H0z'/>"
            "<path d='M3 10h18v10.004c0 .55-.445.996-.993.996H3.993A.994.994 0 0 1 3 20.004V10zm6 "
            "2v2h6v-2H9zM2 4c0-.552.455-1 .992-1h18.016c.548 0 .992.444.992 1v4H2V4z' "
            "fill='%1' opacity='%2'/>"
        "</svg>"
    );

    // RemixIcon: book-fill
    m_svgData["book"] = QString(
        "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' width='24' height='24'>"
            "<path fill='none' d='M0 0h24v24H0z'/>"
            "<path d='M20 22H6.5A3.5 3.5 0 0 1 3 18.5V5a3 3 0 0 1 3-3h14a1 1 0 0 1 1 1v18a1 1 0 0 "
            "1-1 1zm-1-2v-3H6.5a1.5 1.5 0 0 0 0 3H19z' fill='%1' opacity='%2'/>"
        "</svg>"
    );

    // RemixIcon: more-2-fill
    m_svgData["more"] = QString(
        "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' width='24' height='24'>"
            "<path fill='none' d='M0 0h24v24H0z'/><path d='M12 3c-1.1 0-2 .9-2 2s.9 2 2 2 2-.9 2-2"
            "-.9-2-2-2zm0 14c-1.1 0-2 .9-2 2s.9 2 2 2 2-.9 2-2-.9-2-2-2zm0-7c-1.1 0-2 .9-2 2s.9 2 "
            "2 2 2-.9 2-2-.9-2-2-2z' fill='%1' opacity='%2'/>"
        "</svg>"
    );

    // RemixIcon: settings-5-fill
    m_svgData["settings"] = QString(
        "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' width='24' height='24'>"
            "<path fill='none' d='M0 0h24v24H0z'/>"
            "<path d='M2.132 13.63a9.942 9.942 0 0 1 0-3.26c1.102.026 2.092-.502 2.477-1.431.385-."
            "93.058-2.004-.74-2.763a9.942 9.942 0 0 1 2.306-2.307c.76.798 1.834 1.125 2.764.74.93-"
            ".385 1.457-1.376 1.43-2.477a9.942 9.942 0 0 1 3.262 0c-.027 1.102.501 2.092 1.43 2.47"
            "7.93.385 2.004.058 2.763-.74a9.942 9.942 0 0 1 2.307 2.306c-.798.76-1.125 1.834-.74 2"
            ".764.385.93 1.376 1.457 2.477 1.43a9.942 9.942 0 0 1 0 3.262c-1.102-.027-2.092.501-2."
            "477 1.43-.385.93-.058 2.004.74 2.763a9.942 9.942 0 0 1-2.306 2.307c-.76-.798-1.834-1."
            "125-2.764-.74-.93.385-1.457 1.376-1.43 2.477a9.942 9.942 0 0 1-3.262 0c.027-1.102-.50"
            "1-2.092-1.43-2.477-.93-.385-2.004-.058-2.763.74a9.942 9.942 0 0 1-2.307-2.306c.798-.7"
            "6 1.125-1.834.74-2.764-.385-.93-1.376-1.457-2.477-1.43zM12 15a3 3 0 1 0 0-6 3 3 0 0 0"
            " 0 6z' fill='%1' opacity='%2'/>"
        "</svg>"
    );
}

CollettIcons::~CollettIcons() {
    qDebug() << "Destructor: CollettIcons";
}

QIcon CollettIcons::icon(const QString &name) {
    QColor col = qApp->palette().buttonText().color();
    QString svg = m_svgData[name].arg(col.name(QColor::HexRgb)).arg("0.8");
    return QIcon(new SVGIconEngine(svg));
}

} // namespace Collett
