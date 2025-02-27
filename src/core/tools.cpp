/*
** Collett – Core Tools
** ====================
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

#include "tools.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

namespace Collett {


QString JsonUtils::getJsonString(const QJsonObject &object, const QLatin1String &key, QString def) {
    if (object.contains(key)) {
        return object.value(key).toString();
    } else {
        return def;
    }
}

JsonUtilsError JsonUtils::readJson(const QString &filePath, QJsonObject &fileData, bool required) {

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (required) {
            qWarning() << "Could not open file:" << filePath;
            return JsonUtilsError::FileError;
        } else {
            qDebug() << "Missing:" << filePath;
            return JsonUtilsError::NoError;
        }
    }

    QJsonParseError *jsonError = new QJsonParseError();
    QJsonDocument json = QJsonDocument::fromJson(file.readAll(), jsonError);
    if (jsonError->error != QJsonParseError::NoError) {
        qWarning() << "Could not parse file:" << filePath;
        qWarning() << jsonError->errorString();
        return JsonUtilsError::JsonError;
    }
    file.close();

    if (!json.isObject()) {
        qWarning() << "Unexpected content of file:" << filePath;
        return JsonUtilsError::JsonError;
    }

    fileData = json.object();
    qDebug() << "Read:" << filePath;

    return JsonUtilsError::NoError;
}

JsonUtilsError JsonUtils::writeJson(const QString &filePath, const QJsonObject &fileData, bool compact) {

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open file:" << filePath;
        return JsonUtilsError::FileError;
    }
    file.write(QJsonDocument(fileData).toJson(compact ? QJsonDocument::Compact : QJsonDocument::Indented));
    file.close();
    qDebug() << "Wrote:" << filePath;
    return JsonUtilsError::NoError;
}

} // namespace Collett
