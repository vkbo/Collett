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

#include "collett.h"
#include "theme.h"
#include "tools.h"

#include <QString>
#include <QFileInfo>
#include <QJsonObject>
#include <QPalette>
#include <QColor>
#include <QApplication>

using namespace Qt::Literals::StringLiterals;

namespace Collett {

// Constructor/Destructor/Instance
// ===============================

Theme *Theme::staticInstance = nullptr;
Theme *Theme::instance() {
    if (staticInstance == nullptr) {
        staticInstance = new Theme();
        qDebug() << "Constructor: Theme";
    }
    return staticInstance;
}

void Theme::destroy() {
    if (staticInstance != nullptr) {
        qDebug() << "Destructor: Static Theme";
        delete Theme::staticInstance;
    }
}

Theme::Theme(QObject *parent) : QObject(parent) {

    m_settings = Settings::instance();
    this->loadTheme(m_settings->guiTheme());
}

Theme::~Theme() {
    qDebug() << "Destructor: Theme";
}

// Public Methods
// ==============

bool Theme::loadTheme(QString theme) {

    QFileInfo themeFile = QFileInfo(Settings::assetPath("themes").filePath(theme + ".json"));
    if (!themeFile.exists()) return false;
    qInfo() << "Loading Theme:" << theme;

    QJsonObject data;
    if (JsonUtils::readJson(themeFile.absoluteFilePath(), data, true) != JsonUtilsError::NoError) return false;

    QJsonObject jMeta = data.value("c:meta"_L1).toObject();
    QJsonObject jBase = data.value("c:baseColors"_L1).toObject();
    QJsonObject jTheme = data.value("c:themeColors"_L1).toObject();

    // Theme Meta
    m_themeName = JsonUtils::getJsonString(jMeta, "m:name"_L1, "Unknown");

    // Qt Base Colors
    QColor window          = QColor::fromString(JsonUtils::getJsonString(jBase, "window"_L1, "white"));
    QColor windowText      = QColor::fromString(JsonUtils::getJsonString(jBase, "windowText"_L1, "black"));
    QColor base            = QColor::fromString(JsonUtils::getJsonString(jBase, "base"_L1, "white"));
    QColor alternateBase   = QColor::fromString(JsonUtils::getJsonString(jBase, "alternateBase"_L1, "grey"));
    QColor text            = QColor::fromString(JsonUtils::getJsonString(jBase, "text"_L1, "black"));
    QColor toolTipBase     = QColor::fromString(JsonUtils::getJsonString(jBase, "toolTipBase"_L1, "yellow"));
    QColor toolTipText     = QColor::fromString(JsonUtils::getJsonString(jBase, "toolTipText"_L1, "black"));
    QColor button          = QColor::fromString(JsonUtils::getJsonString(jBase, "button"_L1, "white"));
    QColor buttonText      = QColor::fromString(JsonUtils::getJsonString(jBase, "buttonText"_L1, "black"));
    QColor brightText      = QColor::fromString(JsonUtils::getJsonString(jBase, "brightText"_L1, "black"));
    QColor highlight       = QColor::fromString(JsonUtils::getJsonString(jBase, "highlight"_L1, "red"));
    QColor highlightedText = QColor::fromString(JsonUtils::getJsonString(jBase, "highlightedText"_L1, "grey"));
    QColor link            = QColor::fromString(JsonUtils::getJsonString(jBase, "link"_L1, "blue"));
    QColor linkVisited     = QColor::fromString(JsonUtils::getJsonString(jBase, "linkVisited"_L1, "purple"));

    // Theme Colors
    m_colors = {
        QColor::fromString(JsonUtils::getJsonString(jTheme, "root"_L1, "black")),    // ThemeColor::RootColor
        QColor::fromString(JsonUtils::getJsonString(jTheme, "folder"_L1, "yellow")), // ThemeColor::FolderColor
        QColor::fromString(JsonUtils::getJsonString(jTheme, "file"_L1, "grey")),     // ThemeColor::FileColor
        QColor::fromString(JsonUtils::getJsonString(jTheme, "title"_L1, "green")),   // ThemeColor::TitleColor
        QColor::fromString(JsonUtils::getJsonString(jTheme, "chapter"_L1, "red")),   // ThemeColor::ChapterColor
        QColor::fromString(JsonUtils::getJsonString(jTheme, "scene"_L1, "blue")),    // ThemeColor::SceneColor
        QColor::fromString(JsonUtils::getJsonString(jTheme, "note"_L1, "yellow")),   // ThemeColor::NoteColor
        QColor::fromString(JsonUtils::getJsonString(jTheme, "default"_L1, "black")), // ThemeColor::DefaultColor
        QColor::fromString(JsonUtils::getJsonString(jTheme, "faded"_L1, "grey")),    // ThemeColor::FadedColor
        QColor::fromString(JsonUtils::getJsonString(jTheme, "red"_L1, "red")),       // ThemeColor::Red
        QColor::fromString(JsonUtils::getJsonString(jTheme, "orange"_L1, "orange")), // ThemeColor::Orange
        QColor::fromString(JsonUtils::getJsonString(jTheme, "yellow"_L1, "yellow")), // ThemeColor::Yellow
        QColor::fromString(JsonUtils::getJsonString(jTheme, "green"_L1, "green")),   // ThemeColor::Green
        QColor::fromString(JsonUtils::getJsonString(jTheme, "aqua"_L1, "aqua")),     // ThemeColor::Aqua
        QColor::fromString(JsonUtils::getJsonString(jTheme, "blue"_L1, "blue")),     // ThemeColor::Blue
        QColor::fromString(JsonUtils::getJsonString(jTheme, "purple"_L1, "purple")), // ThemeColor::Purple
    };

    // Generate Palette
    QPalette palette;

    palette.setBrush(QPalette::Window, window);
    palette.setBrush(QPalette::WindowText, windowText);
    palette.setBrush(QPalette::Base, base);
    palette.setBrush(QPalette::AlternateBase, alternateBase);
    palette.setBrush(QPalette::Text, text);
    palette.setBrush(QPalette::ToolTipBase, toolTipBase);
    palette.setBrush(QPalette::ToolTipText, toolTipText);
    palette.setBrush(QPalette::Button, button);
    palette.setBrush(QPalette::ButtonText, buttonText);
    palette.setBrush(QPalette::BrightText, brightText);
    palette.setBrush(QPalette::Highlight, highlight);
    palette.setBrush(QPalette::HighlightedText, highlightedText);
    palette.setBrush(QPalette::Link, link);
    palette.setBrush(QPalette::LinkVisited, linkVisited);

    // Derived Colors
    m_isDark = text.lightnessF() > window.lightnessF() ? true : false;

    QColor ref(window);
    if (window.lightnessF() < 0.15) {
        ref = QColor::fromHslF(window.hueF(), window.saturationF(), 0.15, window.alphaF());
    }

    QColor light     = ref.lighter(150);
    QColor mid       = ref.darker(130);
    QColor midLight  = mid.lighter(110);
    QColor dark      = ref.darker(150);
    QColor shadow    = dark.darker(135);
    QColor darkOff   = dark.darker(150);
    QColor shadowOff = ref.darker(150);

    QColor grey   = m_isDark ? QColor(120, 120, 120) : QColor(140, 140, 140);
    QColor dimmed = m_isDark ? QColor(130, 130, 130) : QColor(190, 190, 190);

    QColor placeholder(text);
    placeholder.setAlpha(128);

    palette.setBrush(QPalette::Light, light);
    palette.setBrush(QPalette::Mid, mid);
    palette.setBrush(QPalette::Midlight, midLight);
    palette.setBrush(QPalette::Dark, dark);
    palette.setBrush(QPalette::Shadow, shadow);

    palette.setBrush(QPalette::Disabled, QPalette::Text, dimmed);
    palette.setBrush(QPalette::Disabled, QPalette::WindowText, dimmed);
    palette.setBrush(QPalette::Disabled, QPalette::ButtonText, dimmed);
    palette.setBrush(QPalette::Disabled, QPalette::Base, window);
    palette.setBrush(QPalette::Disabled, QPalette::Dark, darkOff);
    palette.setBrush(QPalette::Disabled, QPalette::Shadow, shadowOff);

    palette.setBrush(QPalette::PlaceholderText, placeholder);

    palette.setBrush(QPalette::Active,   QPalette::Highlight, highlight);
    palette.setBrush(QPalette::Inactive, QPalette::Highlight, highlight);
    palette.setBrush(QPalette::Disabled, QPalette::Highlight, grey);

    QApplication::setPalette(palette);

    return true;
}

} // namespace Collett
