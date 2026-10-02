/*
** Collett - Icon Button Widget
** ============================
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
#include "theme.h"

#include <QSize>
#include <QString>
#include <QToolButton>
#include <QWidget>

namespace Collett {

class MIconButton : public QToolButton
{
    Q_OBJECT

public:
    explicit MIconButton(QWidget *parent = nullptr);
    ~MIconButton();

    // Methods
    void setThemeIcon(const QString &name, ThemeColor color);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override { return sizeHint(); };

public slots:
    void refreshTheme();

private:
    Theme *m_theme;

    // Theme Icon
    QString m_iconName;
    ThemeColor m_iconColor = ThemeColor::DefaultColor;
};
} // namespace Collett
