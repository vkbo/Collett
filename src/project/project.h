/*
** Collett - Project Class
** =======================
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
#include "document.h"
#include "projectdata.h"
#include "storage.h"
#include "tree.h"

#include <QHash>
#include <QJsonObject>
#include <QString>
#include <QTimer>
#include <QtQml/qqmlregistration.h>

namespace Collett {

class Project : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("The project is created on launch")

    Q_PROPERTY(QString name READ name NOTIFY projectChanged)
    Q_PROPERTY(Collett::ProjectModel *model READ model NOTIFY projectChanged)
    Q_PROPERTY(QString lastEditedHandle READ lastEditedHandle NOTIFY projectChanged)

public:
    explicit Project();
    ~Project();

    // Methods
    bool openProject(const QString &path);
    bool saveProject();
    bool saveProjectAs(const QString &path);

    // Document Methods
    Document *openDocument(const QString &handle);
    Q_INVOKABLE QString splitDocument(const QString &handle, int position);
    Q_INVOKABLE bool deleteDocument(const QString &handle);
    Q_INVOKABLE int mergeDocument(const QString &handle);
    bool saveDocument(const QString &handle);
    bool saveOpenDocuments();

    // Getters
    bool isValid() const { return m_isValid; };
    Storage *store() { return m_store; };
    ProjectData *data() { return m_data; };
    Tree *tree() { return m_tree; };

    // Property Getters
    QString name() const { return m_data ? m_data->name() : QString(); };
    ProjectModel *model() const { return m_tree ? m_tree->model() : nullptr; };
    QString lastEditedHandle() const { return m_data ? m_data->lastEditedHandle() : QString(); };

    // Property Setters
    Q_INVOKABLE void setLastEditedHandle(const QString &handle);

    // Error Handling
    bool hasError() const { return !m_lastError.isEmpty(); };
    QString lastError() const { return m_lastError; };

signals:
    void projectChanged();

private:
    bool m_isValid = false;
    QString m_lastError = "";

    Storage *m_store = nullptr;
    ProjectData *m_data = nullptr;
    Tree *m_tree = nullptr;

    // Document Cache
    QHash<QString, Document *> m_documents;
    QTimer *m_autoSaveTimer = nullptr;

private slots:
    void onAutoSave();
};
} // namespace Collett
