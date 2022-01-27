/*
** Collett – GUI Text Editor Class
** ===============================
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

#ifndef GUI_TEXTEDIT_H
#define GUI_TEXTEDIT_H

#include "collett.h"
#include "settings.h"
#include "data.h"

#include <QObject>
#include <QWidget>
#include <QTextEdit>
#include <QJsonObject>
#include <QTextCharFormat>
#include <QTextBlockFormat>

namespace Collett {

class GuiTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    GuiTextEdit(QWidget *parent=nullptr);
    ~GuiTextEdit() {};

    QJsonObject toJsonObject();
    void setJsonObject(const QJsonObject &json);

public slots:
    void applyDocAction(DocAction action);

private:
    CollettSettings::TextFormat m_format;

    void initFormats();

};
} // namespace Collett

#endif // GUI_TEXTEDIT_H
