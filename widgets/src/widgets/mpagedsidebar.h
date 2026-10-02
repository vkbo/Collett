/*
** Collett - Paged Side Bar Widget
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

#include <QAbstractButton>
#include <QAction>
#include <QButtonGroup>
#include <QColor>
#include <QLabel>
#include <QList>
#include <QPaintEvent>
#include <QSize>
#include <QString>
#include <QToolBar>
#include <QToolButton>
#include <QWidget>

namespace Collett {

class MPagedToolButton;
class MPagedToolLabel;

class MPagedSideBar : public QToolBar
{
    Q_OBJECT

public:
    explicit MPagedSideBar(QWidget *parent = nullptr);
    ~MPagedSideBar();

    // Getters
    int selected() const { return m_group->checkedId(); };
    MPagedToolButton *button(int buttonId) const;

    // Setters
    void setLabelColor(const QColor &color);
    void setSelected(int buttonId);

    // Methods
    void addLabel(const QString &text);
    void addButton(const QString &text, int buttonId);

signals:
    void buttonClicked(int buttonId);

private:
    QButtonGroup *m_group;
    QAction *m_stretchAction;
    QColor m_labelColor;
    QList<MPagedToolLabel *> m_labels;

private slots:
    void onButtonClicked(QAbstractButton *button);
};

class MPagedToolButton : public QToolButton
{
    Q_OBJECT

public:
    explicit MPagedToolButton(QWidget *parent = nullptr);
    ~MPagedToolButton();

    // Methods
    QSize sizeHint() const override;

protected:
    // Events
    void paintEvent(QPaintEvent *event) override;

private:
    // Layout
    int m_buttonHeight;
    int m_textMargin;
    int m_arrowSize;
};

class MPagedToolLabel : public QLabel
{
    Q_OBJECT

public:
    explicit MPagedToolLabel(const QString &text, QWidget *parent = nullptr);
    ~MPagedToolLabel();

    // Setters
    void setTextColor(const QColor &color);

protected:
    // Events
    void paintEvent(QPaintEvent *event) override;

private:
    QColor m_textColor;

    // Layout
    int m_textMargin;
};
} // namespace Collett
