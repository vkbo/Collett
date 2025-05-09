/*
** Collett – Project Class
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

#ifndef COLLETT_PROJECT_H
#define COLLETT_PROJECT_H

#include "collett.h"
#include "projectdata.h"
#include "storage.h"
#include "tree.h"

#include <QJsonObject>

namespace Collett {

class Project : public QObject
{
    Q_OBJECT

public:
    explicit Project();
    ~Project();

    // Methods
    bool openProject(const QString &path);
    bool saveProject();
    bool saveProjectAs(const QString &path);

    // Getters
    bool isValid() const {return m_isValid;};
    Storage *store() {return m_store;};
    ProjectData *data() {return m_data;};
    Tree *tree() {return m_tree;};

    // Error Handling
    bool hasError() const {return !m_lastError.isEmpty();};
    QString lastError() const {return m_lastError;};

private:
    bool     m_isValid = false;
    QString  m_lastError = "";

    Storage     *m_store = nullptr;
    ProjectData *m_data = nullptr;
    Tree        *m_tree = nullptr;

};
} // namespace Collett

#endif // COLLETT_PROJECT_H
