/*
** Collett – Main Theme Class
** ==========================
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

#ifndef COLLETT_THEME_H
#define COLLETT_THEME_H

#include "collett.h"
#include "settings.h"
#include "icons.h"

#include <QString>
#include <QColor>
#include <QList>

namespace Collett {

class Theme : public QObject
{
    Q_OBJECT

public:
    static Theme *instance();
    static void destroy();

    explicit Theme(QObject *parent = nullptr);
    ~Theme();

    // Getters
    bool isDark() const {return m_isDark;};
    QColor getColor(ThemeColor color) const {return m_colors.at(color);};

    // Methods
    bool loadTheme(QString theme);

private:
    static Theme *staticInstance;
    Settings *m_settings;
    Icons    *m_icons;

    // Meta
    QString m_name = "";
    QString m_author = "";
    QString m_credit = "";
    QString m_license = "";

    // Colors
    bool m_isDark = false;
    QList<QColor> m_colors = {
        QColor::fromString("black"),  // ThemeColor::RootColor
        QColor::fromString("yellow"), // ThemeColor::Folder Color
        QColor::fromString("grey"),   // ThemeColor::FileColor
        QColor::fromString("green"),  // ThemeColor::TitleColor
        QColor::fromString("red"),    // ThemeColor::ChapterColor
        QColor::fromString("blue"),   // ThemeColor::SceneColor
        QColor::fromString("yellow"), // ThemeColor::NoteColor
        QColor::fromString("black"),  // ThemeColor::DefaultColor
        QColor::fromString("grey"),   // ThemeColor::FadedColor
        QColor::fromString("red"),    // ThemeColor::Red
        QColor::fromString("orange"), // ThemeColor::Orange
        QColor::fromString("yellow"), // ThemeColor::Yellow
        QColor::fromString("green"),  // ThemeColor::Green
        QColor::fromString("aqua"),   // ThemeColor::Aqua
        QColor::fromString("blue"),   // ThemeColor::Blue
        QColor::fromString("purple"), // ThemeColor::Purple
    };

    friend class Icons;
};
} // namespace Collett

#endif // COLLETT_THEME_H
