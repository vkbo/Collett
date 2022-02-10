/*
** Collett – Document Class
** ========================
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

#ifndef COLLETT_DOCUMENT_H
#define COLLETT_DOCUMENT_H

#include "collett.h"
#include "storage.h"

#include <QUuid>
#include <QObject>
#include <QJsonArray>

namespace Collett {

class Document : public QObject
{
    Q_OBJECT

public:
    explicit Document(Storage *store, const QUuid uuid);
    ~Document() {};

    // Setters

    void setContent(const QJsonArray &content);
    void setLocked(bool locked);

    // Getters

    QJsonArray content() const;
    QUuid handle() const;

    // Check State

    bool isEmpty() const;
    bool isUnsaved() const;

    // Methods

    bool read();
    bool write();

private:
    Storage *m_store;
    QUuid    m_handle;

    bool m_locked;
    bool m_unsaved;

    // Data Variables

    QString    m_created;
    QString    m_updated;
    QJsonArray m_content;

    void docChanged();

};
} // namespace Collett

#endif // COLLETT_DOCUMENT_H
