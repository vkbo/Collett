/*
Collett – GUI Main Tool Bar Class
=================================

This file is a part of Collett
Copyright 2020–2021, Veronica Berglyd Olsen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef GUIMAINTOOLBAR_H
#define GUIMAINTOOLBAR_H

#include <QObject>
#include <QToolBar>
#include <QLabel>
#include <QString>

namespace Collett {

class GuiMainToolBar : public QToolBar
{
    Q_OBJECT

public:
    GuiMainToolBar(QWidget *parent=nullptr);
    ~GuiMainToolBar() {};

    void setProjectName(const QString &name);

private:
    QLabel *m_projectName;

};
} // namespace Collett

#endif // GUIMAINTOOLBAR_H
