/*
** Collett - Project Data Class
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

#pragma once

#include "collett.h"

#include <QJsonObject>
#include <QString>

namespace Collett {

class ProjectData : public QObject
{
    Q_OBJECT

public:
    explicit ProjectData(QObject *parent = nullptr);
    ~ProjectData();

    // Methods
    void pack(QJsonObject &data);
    void unpack(const QJsonObject &data);

    // Getters
    QString name() const { return m_projectName; };
    QString lastEditedHandle() const { return m_lastEditedHandle; };
    QString spellLanguage() const { return m_spellLanguage; };
    bool hasSpellLanguage() const { return !m_spellLanguage.isEmpty(); };

    // Setters
    void setLastEditedHandle(const QString &handle) { m_lastEditedHandle = handle; };
    void setSpellLanguage(const QString &language) { m_spellLanguage = language.trimmed(); };

private:
    QString m_createdTime = "";
    QString m_projectName = "";
    QString m_lastEditedHandle = "";

    // The project's spell check language. Empty means no override, so the
    // global setting applies. Stored as null in the project file in that case.
    QString m_spellLanguage = "";
};
} // namespace Collett
