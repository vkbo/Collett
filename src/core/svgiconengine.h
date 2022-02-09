/*
** Collett – Core SVG Icon Engine
** ==============================
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

#ifndef SVGICONENGINE_H
#define SVGICONENGINE_H

#include <QIcon>
#include <QRect>
#include <QSize>
#include <QPixmap>
#include <QPainter>
#include <QByteArray>
#include <QIconEngine>

namespace Collett {

class SVGIconEngine : public QIconEngine
{

public:
    explicit SVGIconEngine(const QByteArray &iconBuffer);

    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;
    QIconEngine *clone() const override;
    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;

private:
    QByteArray m_iconData;

};
} // namespace Collett

#endif // SVGICONENGINE_H
