/*
** Collett – Core SVG Icon Engine
** ==============================
**
** This file is a part of Collett
** Copyright 2021–2022, Veronica Berglyd Olsen
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

#include "svgiconengine.h"

#include <QIcon>
#include <QRect>
#include <QSize>
#include <QDebug>
#include <QPixmap>
#include <QPainter>
#include <QByteArray>
#include <QSvgRenderer>

namespace Collett {

/**
 * Custom Icon Engine
 * ==================
 * Based on: https://stackoverflow.com/a/44757951
 */

SVGIconEngine::SVGIconEngine(const QByteArray &iconBuffer) : m_iconData(iconBuffer) {}

void SVGIconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) {
    QSvgRenderer renderer(m_iconData);
    renderer.render(painter, rect);
}

QIconEngine *SVGIconEngine::clone() const {
    return new SVGIconEngine(*this);
}

QPixmap SVGIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) {

    QImage img(size, QImage::Format_ARGB32);
    img.fill(qRgba(0, 0, 0, 0));
    QPixmap pix = QPixmap::fromImage(img, Qt::NoFormatConversion);
    {
        QPainter painter(&pix);
        QRect rect(QPoint(0.0, 0.0), size);
        this->paint(&painter, rect, mode, state);
    }
    return pix;
}

} // namespace Collett
