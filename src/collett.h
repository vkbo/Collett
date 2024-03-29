/*
** Collett – Collett Main Header
** =============================
**
** This file is a part of Collett
** Copyright 2020–2023, Veronica Berglyd Olsen
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

namespace Collett {

enum Severity {
    Information,
    Warning,
    Error,
    Bug,
};

} // namespace Collett

#endif // COLLETT_H
