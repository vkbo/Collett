/*
** Collett – Core Storage Class
** ============================
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

#ifndef COLLETT_STORAGE_H
#define COLLETT_STORAGE_H

#include "collett.h"

#include <QDir>
#include <QJsonObject>

namespace Collett {

class Storage : public QObject
{
    Q_OBJECT

public:
    explicit Storage(const QString &path, bool compact=false);
    ~Storage();

    // Methods
    bool readProject(QJsonObject &fileData);
    bool writeProject(const QJsonObject &fileData);
    bool readStructure(QJsonObject &fileData);
    bool writeStructure(const QJsonObject &fileData);

    // Getters
    bool isValid() const {return m_isValid;};
    QString projectPath() const;

    // Error Handling
    bool hasError() const {return !m_lastError.isEmpty();};
    QString lastError() const {return m_lastError;};

    // Static Methods
    static QString getJsonString(const QJsonObject &object, const QLatin1String &key, QString def);

private:
    bool readJson(const QString &filePath, QJsonObject &fileData, bool required);
    bool writeJson(const QString &filePath, const QJsonObject &fileData);
    void writeCollett();

    QDir m_rootPath;
    QDir m_projectDir;
    QDir m_contentDir;
    bool m_compactJson;

    bool m_isValid = false;
    QString m_lastError = "";

};
} // namespace Collett

#endif // COLLETT_STORAGE_H
