/*
** Collett – Collett Main Header
** =============================
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

#ifndef COLLETT_H
#define COLLETT_H

#define COL_VERSION_STR  "0.0.1-alpha0"
#define COL_VERSION_NUM  0x000001a0
#define COL_VERSION_DATE "2021-11-14"

#include <QLatin1String>

namespace Collett {

    const QLatin1String ColNsCollett = QLatin1String("urn:collett:names:collettdoc:xmlns:collett:1.0");
    const QLatin1String ColNsConfig  = QLatin1String("urn:collett:names:collettdoc:xmlns:config:1.0");
    const QLatin1String ColNsMeta    = QLatin1String("urn:collett:names:collettdoc:xmlns:meta:1.0");
    const QLatin1String ColNsStyle   = QLatin1String("urn:collett:names:collettdoc:xmlns:style:1.0");
    const QLatin1String ColNsText    = QLatin1String("urn:collett:names:collettdoc:xmlns:text:1.0");

    const QLatin1String XmlNsDC      = QLatin1String("http://purl.org/dc/elements/1.1/");

} // namespace Collett

#endif // COLLETT_H
