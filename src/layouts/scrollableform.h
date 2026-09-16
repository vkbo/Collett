/*
** Collett - Scrollable Form Layout
** ================================
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

#include <QList>
#include <QPaintEvent>
#include <QScrollArea>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

namespace Collett {

class MSettingsGroup;

class MScrollableForm : public QScrollArea
{
    Q_OBJECT

public:
    explicit MScrollableForm(QWidget *parent = nullptr);
    ~MScrollableForm();

    // Getters
    int groupCount() const { return m_groups.count(); };
    int rowCount() const;

    // Methods
    void addGroupLabel(const QString &text);
    void addRow(const QString &label, QWidget *widget, const QString &helpText = QString());
    void finalise();

private:
    QWidget *m_content;
    QVBoxLayout *m_layout;
    QList<MSettingsGroup *> m_groups;

    // Helpers
    MSettingsGroup *currentGroup();
};

class MSettingsGroup : public QWidget
{
    Q_OBJECT

public:
    explicit MSettingsGroup(QWidget *parent = nullptr);
    ~MSettingsGroup();

    // Getters
    int rowCount() const { return m_rows.count(); };

    // Methods
    void addRow(const QString &label, QWidget *widget, const QString &helpText);

protected:
    // Events
    void paintEvent(QPaintEvent *event) override;

private:
    QVBoxLayout *m_layout;
    QList<QWidget *> m_rows;
};
} // namespace Collett
