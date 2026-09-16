/*
** Collett - Main Theme Class
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

#pragma once

#include "collett.h"
#include "settings.h"
#include "icons.h"

#include <QColor>
#include <QDir>
#include <QList>
#include <QSize>
#include <QString>

namespace Collett {

struct ThemeEntry
{
    QString key;
    QString name;
    bool dark = false;
    QString path;
};

class MPushButton;
class Theme : public QObject
{
    Q_OBJECT

public:
    static Theme *instance();
    static void destroy();

    explicit Theme(QObject *parent = nullptr);
    ~Theme();

    // Getters
    bool isDark() const { return m_isDark; };
    QColor getColor(ThemeColor color) const { return m_colors.at(color); };
    QColor getSyntaxColor(SyntaxColor color) const { return m_syntaxColors.at(color); };
    QColor accentColor() const { return m_accentColor; };
    QColor helpTextColor() const { return m_helpTextColor; };
    Icons *icons() const { return m_icons; };
    QString currentTheme() const { return m_currentTheme; };
    QList<ThemeEntry> themes() const { return m_themes; };
    QList<ThemeEntry> lightThemes() const;
    QList<ThemeEntry> darkThemes() const;
    bool hasTheme(const QString &key) const;
    bool isDesktopDarkMode() const;

    qreal fontPointSizeF() const { return m_fontPointSizeF; };
    int fontPixelSize() const { return m_fontPixelSize; };
    int baseIconHeight() const { return m_baseIconHeight; };
    int baseButtonHeight() const { return m_baseButtonHeight; };
    QSize baseIconSize() const { return m_baseIconSize; };
    QSize buttonIconSize() const { return m_buttonIconSize; };
    QSize toolButtonSize() const { return m_toolButtonSize; };

    // Methods
    void scanThemes(const QDir &dir);
    bool loadTheme();
    bool loadTheme(const QString &key);
    MPushButton *getStandardButton(StandardButton button, QWidget *parent) const;

signals:
    void themeChanged();

private:
    static Theme *staticInstance;
    Settings *m_settings;
    Icons *m_icons;

    // Syntax Colors
    QList<QColor> m_syntaxColors = {
        QColor::fromString("blue"),   // SyntaxColor::SyntaxHeader
        QColor::fromString("green"),  // SyntaxColor::SyntaxEmphasis
        QColor::fromString("grey"),   // SyntaxColor::SyntaxComment
        QColor::fromString("red"),    // SyntaxColor::SyntaxSpellLine
        QColor::fromString("orange"), // SyntaxColor::SyntaxErrorLine
    };

    // Themes
    QList<ThemeEntry> m_themes;
    QString m_currentTheme;

    // Meta
    QString m_name = "";
    QString m_author = "";
    QString m_credit = "";
    QString m_license = "";

    // Colors
    bool m_isDark = false;
    QColor m_accentColor = QColor::fromString("purple");
    QColor m_helpTextColor = QColor::fromString("grey");
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
        QColor::fromString("black"),  // ThemeColor::ToolColor
        QColor::fromString("green"),  // ThemeColor::AcceptColor
        QColor::fromString("red"),    // ThemeColor::RejectColor
        QColor::fromString("blue"),   // ThemeColor::ActionColor
        QColor::fromString("orange"), // ThemeColor::OptionColor
        QColor::fromString("green"),  // ThemeColor::ApplyColor
        QColor::fromString("yellow"), // ThemeColor::CreateColor
        QColor::fromString("grey"),   // ThemeColor::DestroyColor
        QColor::fromString("green"),  // ThemeColor::ResetColor
        QColor::fromString("green"),  // ThemeColor::AddColor
        QColor::fromString("green"),  // ThemeColor::ChangeColor
        QColor::fromString("red"),    // ThemeColor::RemoveColor
    };

    // Size Info
    qreal m_fontPointSizeF = 12.0;
    int m_fontPixelSize = 16;
    int m_baseIconHeight = 16;
    int m_baseButtonHeight = 16;
    QSize m_baseIconSize = {16, 16};
    QSize m_buttonIconSize = {14, 14};
    QSize m_toolButtonSize = {32, 32};

    friend class Icons;

private slots:
    void onColorSchemeChanged();
};
} // namespace Collett
