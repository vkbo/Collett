/*
** Collett - Core Tools
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
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QFontInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

using namespace Qt::Literals::StringLiterals;

namespace Collett {

// Font Utils
// ==========

/**! @brief Describe a font for display, like "12 pt Sans Serif Bold".
 *
 * The family and style are the ones asked for, as the system may resolve
 * them to another font. Style words already in the family name are left out.
 */
QString FontUtils::describeFont(const QFont &font)
{
    const QFontInfo info(font);
    const QString family = font.family().isEmpty() ? info.family() : font.family();
    const QString style = font.styleName().isEmpty() ? info.styleName() : font.styleName();
    QStringList parts = {QStringLiteral("%1 pt").arg(qRound(font.pointSizeF() > 0.0 ? font.pointSizeF() : info.pointSizeF())), family};
    for (const QString &word : style.split(u' ', Qt::SkipEmptyParts)) {
        if (!family.contains(word)) parts.append(word);
    }
    return parts.join(u' ');
}

// JSON Utils
// ==========

QString JsonUtils::getJsonString(const QJsonObject &object, const QLatin1String &key, QString def)
{
    if (object.contains(key)) {
        return object.value(key).toString();
    } else {
        return def;
    }
}

JsonUtilsError JsonUtils::readJson(const QString &filePath, QJsonObject &fileData, bool required)
{

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

/**! @brief Write a JSON object to file.
 *
 * Unless compact is set, the output is indented by jsonEncode up to level
 * nmax.
 */
JsonUtilsError JsonUtils::writeJson(const QString &filePath, const QJsonObject &fileData, bool compact, int nmax)
{

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open file:" << filePath;
        return JsonUtilsError::FileError;
    }
    file.write(compact ? QJsonDocument(fileData).toJson(QJsonDocument::Compact) : jsonEncode(fileData, nmax));
    file.close();
    qDebug() << "Wrote:" << filePath;
    return JsonUtilsError::NoError;
}

/**! @brief The meta block of a file: the version that wrote it, when it
 * was created, and when it was written, which is now.
 */
QJsonObject JsonUtils::packMeta(const QString &created)
{
    QJsonObject meta;
    meta["m:version"_L1] = QString(COL_VERSION_STR);
    meta["m:created"_L1] = created;
    meta["m:updated"_L1] = QDateTime::currentDateTime().toString(Qt::ISODate);
    return meta;
}

/**! @brief The created time from the meta block of a file, or def if there
 * is none.
 */
QString JsonUtils::unpackCreated(const QJsonObject &data, const QString &def)
{
    return getJsonString(data.value("c:meta"_L1).toObject(), "m:created"_L1, def);
}

/**! @brief Encode a JSON object with two-space indentation up to level nmax.
 *
 * Containers deeper than nmax are written on a single line. If nmax is 0 or
 * less, all levels are indented. The compact output from Qt is re-formatted,
 * so escaping and key order are the same as for QJsonDocument.
 */
QByteArray JsonUtils::jsonEncode(const QJsonObject &data, int nmax)
{
    const QByteArray json = QJsonDocument(data).toJson(QJsonDocument::Compact);

    QByteArray out;
    out.reserve(json.size() + json.size() / 4);

    int n = 0;
    bool inString = false;
    bool escaped = false;
    auto expand = [nmax](int level) { return nmax <= 0 || level <= nmax; };
    auto newline = [&out, &n]() { out.append('\n').append(2 * n, ' '); };

    for (qsizetype i = 0; i < json.size(); ++i) {
        const char c = json.at(i);
        if (inString) {
            out.append(c);
            if (escaped) escaped = false;
            else if (c == '\\') escaped = true;
            else if (c == '"') inString = false;
            continue;
        }
        switch (c) {
        case '"':
            inString = true;
            out.append(c);
            break;
        case '{':
        case '[':
            out.append(c);
            if (i + 1 < json.size() && (json.at(i + 1) == '}' || json.at(i + 1) == ']')) {
                out.append(json.at(++i));
            } else if (expand(++n)) {
                newline();
            }
            break;
        case '}':
        case ']':
            if (expand(n--)) newline();
            out.append(c);
            break;
        case ',':
            out.append(',');
            if (expand(n)) newline();
            else out.append(' ');
            break;
        case ':':
            out.append(": ");
            break;
        default:
            out.append(c);
            break;
        }
    }
    out.append('\n');

    return out;
}

} // namespace Collett
