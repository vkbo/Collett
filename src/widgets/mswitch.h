/*
** Collett - Toggle Switch Widget
** ==============================
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

#include <QAbstractButton>
#include <QPaintEvent>
#include <QPropertyAnimation>
#include <QSize>
#include <QWidget>

namespace Collett {

class MSwitch : public QAbstractButton
{
    Q_OBJECT
    Q_PROPERTY(int offset READ offset WRITE setOffset)

public:
    explicit MSwitch(QWidget *parent = nullptr, int height = 0);
    ~MSwitch();

    // Getters
    int offset() const { return m_offset; };

    // Setters
    void setOffset(int offset);

    // Methods
    QSize sizeHint() const override { return QSize(m_width, m_height); };
    QSize minimumSizeHint() const override { return sizeHint(); };

protected:
    // Events
    void paintEvent(QPaintEvent *event) override;
    void checkStateSet() override;
    void nextCheckState() override;

private:
    QPropertyAnimation *m_anim;

    // Layout
    int m_height;
    int m_width;
    int m_radius;
    int m_knobSize;
    int m_knobRadius;
    int m_offset;

    // Helpers
    int targetOffset() const { return this->isChecked() ? m_width - m_radius : m_radius; };
};
} // namespace Collett
