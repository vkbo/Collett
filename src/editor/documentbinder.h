/*
** Collett - Document Binder Class
** ===============================
**
** This file is a part of Collett
** Copyright (C) 2026 Veronica Berglyd Olsen
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
#include "document.h"
#include "project.h"

#include <QObject>
#include <QPointer>
#include <QQuickItem>
#include <QString>
#include <QtQml/qqmlregistration.h>

namespace Collett {

class DocumentBinder : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QQuickItem *target READ target WRITE setTarget NOTIFY targetChanged)
    Q_PROPERTY(Collett::Project *project READ project WRITE setProject NOTIFY projectChanged)
    Q_PROPERTY(QString handle READ handle WRITE setHandle NOTIFY handleChanged)

public:
    explicit DocumentBinder(QObject *parent = nullptr);
    ~DocumentBinder();

    // Getters
    QQuickItem *target() const { return m_target; };
    Project *project() const { return m_project; };
    QString handle() const { return m_handle; };

    // Setters
    void setTarget(QQuickItem *target);
    void setProject(Project *project);
    void setHandle(const QString &handle);

signals:
    void targetChanged();
    void projectChanged();
    void handleChanged();

private:
    QPointer<QQuickItem> m_target;
    QPointer<Project> m_project;
    QString m_handle = "";
    QPointer<Document> m_document;

    void openDocument();
    void bindDocument(Document *document);
};
} // namespace Collett
