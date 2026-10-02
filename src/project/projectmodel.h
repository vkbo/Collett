/*
** Collett - Project Model Class
** =============================
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
#include "group.h"

#include <QAbstractListModel>
#include <QByteArray>
#include <QHash>
#include <QList>
#include <QModelIndex>
#include <QString>
#include <QVariant>
#include <QtQml/qqmlregistration.h>

namespace Collett {

/**! @brief A list model of the documents in one project group.
 *
 * The documents are listed in reading order. Folding a partition or chapter
 * does not remove rows, it marks the rows it covers as hidden, so the view
 * can animate them in and out.
 */
class ProjectModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("The project model is provided by the project")

public:
    enum Roles
    {
        HandleRole = Qt::UserRole + 1,
        TitleRole,
        LevelRole,
        WordsRole,
        ExpandedRole,
        FoldableRole,
        HiddenRole,
    };
    Q_ENUM(Roles)

    explicit ProjectModel(QObject *parent = nullptr);
    ~ProjectModel();

    // Model Interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Methods
    void setGroup(Group *group);
    Q_INVOKABLE void toggleExpanded(int row);
    Q_INVOKABLE int rowOf(const QString &handle) const;
    Q_INVOKABLE void setTitle(int row, const QString &title);

signals:
    void structureChanged();

private:
    Group *m_group = nullptr;
    QList<bool> m_hidden;
    QList<bool> m_foldable;

    void updateStructure();
};
} // namespace Collett
