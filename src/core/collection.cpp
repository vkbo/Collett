/*
** Collett – Collection Class
** ==========================
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

#include "collett.h"
#include "collection.h"

#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QFileInfo>
#include <QIODevice>
#include <QByteArray>
#include <QJsonArray>
#include <QApplication>
#include <QJsonDocument>
#include <QLatin1String>
#include <QJsonParseError>

namespace Collett {

/**
 * Class Constructor/Destructor
 * ============================
 */

Collection::Collection() {

    qDebug() << "Constructor: Collection";

    m_path = "";
    m_lastError = "";
    m_changed = false;

    m_created = QDateTime::currentDateTime().toString(Qt::ISODate);
    m_updated = m_created;

    m_name = tr("Unnamed Collection");
}

Collection::~Collection() {
    qDebug() << "Destructor: Collection";
}

/**
 * Class Methods
 * =============
 */

void Collection::openCollection(const QString &path) {

    QFileInfo fInfo(path);
    if (!fInfo.isFile()) {
        m_lastError = tr("The provided path is not a file.");
        return;
    }
    if (fInfo.suffix() != "collett-collection") {
        m_lastError = tr("The file is not a Collett collection file.");
        return;
    }

    QJsonObject json;
    if (!this->jsonDocumentReader(path, json)) {
        m_lastError = tr("Could not read file: %1").arg(path);
        return;
    }
    m_path = path;
    qInfo() << "Loading collection from:" << m_path;

    QString fileFormat;
    if (json.contains(QLatin1String("$:format"))) {
        fileFormat = json.value(QLatin1String("$:format")).toString();
    }
    if (json.contains(QLatin1String("m:created"))) {
        m_created = json.value(QLatin1String("m:created")).toString();
    } else {
        m_created = "unknown";
    }
    if (json.contains(QLatin1String("m:updated"))) {
        m_updated = json.value(QLatin1String("m:updated")).toString();
    } else {
        m_created = "unknown";
    }
    if (json.contains(QLatin1String("u:name"))) {
        m_name = json.value(QLatin1String("u:name")).toString();
    } else {
        m_name = tr("Unnamed Collection");
    }
}

void Collection::saveCollection() {

    if (m_path.isEmpty()) {
        return;
    }

    qInfo() << "Saving collection to:" << m_path;

    QJsonObject json;
    QJsonArray jContent;

    json.insert(QLatin1String("$:format"), "CollettCollection:0.1");
    json.insert(QLatin1String("$:version"), qApp->applicationVersion());
    json.insert(QLatin1String("m:created"), m_created);
    json.insert(QLatin1String("m:updated"), m_updated);
    json.insert(QLatin1String("u:name"), m_name);
    json.insert(QLatin1String("x:content"), jContent);

    if (!this->jsonDocumentWriter(m_path, json, false)) {
        m_lastError = tr("Could not write file: %1").arg(m_path);
    }
}

/**!
 * @brief Read a JSON document from path.
 * 
 * @param filePath The path to the JSON file.
 * @param fileData The JSON object to load the data into.
 * @return If successful, returns true, otherwise false.
 */
bool Collection::jsonDocumentReader(const QString &filePath, QJsonObject &fileData) {

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
bool Collection::jsonDocumentWriter(const QString &filePath, const QJsonObject &fileData, bool compact) {

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

/**
 * Class Setters
 * =============
 */

void Collection::setCollectionName(const QString &name) {
    m_name = name.simplified();
}

/**
 * Class Getters
 * =============
 */

QString Collection::collectionName() const {
    return m_name;
}

QString Collection::relativePath(const QString &path) const {
    return QFileInfo(m_path).dir().filePath(path);
}

bool Collection::hasError() const {
    return !m_lastError.isEmpty();
}

QString Collection::lastError() const {
    return m_lastError;
}

} // namespace Collett
