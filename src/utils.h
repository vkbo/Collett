/*
** Collett – Collett Utils File
** ============================
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

#ifndef COLLETT_UTILS_H
#define COLLETT_UTILS_H

#include <QFile>
#include <QString>
#include <QIODevice>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>

namespace Collett {

/**!
 * @brief Read a JSON document from path.
 * 
 * @param filePath The path to the JSON file.
 * @param fileData The JSON object to load the data into.
 * @return If successful, returns true, otherwise false.
 */
bool jsonDocumentReader(const QString &filePath, QJsonObject &fileData) {

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open file:" << filePath;
        return false;
    }

    QJsonParseError *error = new QJsonParseError();
    QJsonDocument json = QJsonDocument::fromJson(file.readAll(), error);
    if (error->error != QJsonParseError::NoError) {
        qWarning() << "Could not parse file:" << filePath;
        qWarning() << error->errorString();
        return false;
    }

    if (!json.isObject()) {
        qWarning() << "Unexpected content of file:" << filePath;
        return false;
    }

    fileData = json.object();

    return true;
}

/**!
 * @brief Write a JSON document to path.
 * 
 * @param filePath The path to the JSON file.
 * @param fileData The JSON object to write to the file.
 * @param compact  Whether the JSON format should be compact or not.
 * @return If successful, returns true, otherwise false.
 */
bool jsonDocumentWriter(const QString &filePath, const QJsonObject &fileData, bool compact) {

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open file:" << filePath;
        return false;
    }

    QJsonDocument doc(fileData);
    QByteArray jsonData = doc.toJson(compact ? QJsonDocument::Compact : QJsonDocument::Indented);

    if (compact) {
        file.write(jsonData);
    } else {
        for (QByteArray line: jsonData.split('\n')) {
            QByteArray trimmed = line.trimmed();
            if (trimmed.length() > 0) {
                file.write(QByteArray((line.length() - trimmed.length())/4, '\t') + trimmed + '\n');
            }
        }
    }
    file.close();

    return true;
}

} // namespace Collett

#endif // COLLETT_UTILS_H
