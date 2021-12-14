/*
** Collett – Data Utilities
** ========================
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

#include "datautils.h"

#include <QString>
#include <QJsonObject>

namespace Collett {

/**!
 * @brief Get string from JSON object.
 * 
 * @param object the json object.
 * @param key    the key of the value to look up.
 * @param def    the default value to return in case the key does not exist.
 * @return the value or the default as a string.
 */
QString DataUtils::getJsonString(const QJsonObject &object, const QString &key, QString def) {
    if (object.contains(key)) {
        return object.value(key).toString();
    } else {
        return def;
    }
}

} // namespace Collett
