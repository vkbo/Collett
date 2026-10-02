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

#include "collett.h"
#include "theme.h"
#include "tools.h"
#include "icons.h"
#include "miconbutton.h"
#include "mpushbutton.h"

#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QGuiApplication>
#include <QStyle>
#include <QStyleHints>
#include <QJsonObject>
#include <QPalette>
#include <QColor>
#include <QApplication>
#include <QCoreApplication>
#include <QFontMetrics>
#include <QSize>

#include <algorithm>

using namespace Qt::Literals::StringLiterals;

namespace Collett {

namespace {
// Label, icon key and icon colour of each standard button, indexed by the
// StandardButton enum. The labels share one translation context so each
// only needs translating once.
struct StandardButtonDef
{
    const char *text;
    const char *icon;
    ThemeColor color;
};
constexpr StandardButtonDef standardButtons[] = {
    {QT_TRANSLATE_NOOP("Button", "Save"), "btn_save", ThemeColor::ActionColor},     // StandardButton::SaveButton
    {QT_TRANSLATE_NOOP("Button", "Cancel"), "btn_cancel", ThemeColor::RejectColor}, // StandardButton::CancelButton
    {QT_TRANSLATE_NOOP("Button", "OK"), "btn_ok", ThemeColor::ActionColor},         // StandardButton::OkButton
};
// Tool tip, icon key and icon colour of each tool button, indexed by the
// ToolButton enum.
constexpr StandardButtonDef toolButtons[] = {
    {QT_TRANSLATE_NOOP("Button", "Select Font"), "font", ThemeColor::ToolColor}, // ToolButton::FontButton
};
} // namespace

// Constructor/Destructor/Instance
// ===============================

Theme *Theme::staticInstance = nullptr;
Theme *Theme::instance()
{
    if (staticInstance == nullptr) {
        staticInstance = new Theme();
        qDebug() << "Constructor: Theme";
    }
    return staticInstance;
}

void Theme::destroy()
{
    if (staticInstance != nullptr) {
        qDebug() << "Destructor: Static Theme";
        delete Theme::staticInstance;
        Theme::staticInstance = nullptr;
    }
}

Theme::Theme(QObject *parent) : QObject(parent)
{

    m_settings = Settings::instance();
    m_icons = new Icons(this);

    this->scanThemes(Settings::assetPath("themes"));
    this->loadTheme();
    m_icons->loadIcons(m_settings->iconSet());

    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, &Theme::onColorSchemeChanged);

    QFontMetrics metric(QApplication::font());
    m_fontPointSizeF = QApplication::font().pointSizeF();
    m_fontPixelSize = metric.height();
    m_baseIconHeight = metric.ascent();
    m_baseButtonHeight = int(round(1.35 * metric.ascent()));
    m_baseIconSize = QSize(m_baseIconHeight, m_baseIconHeight);
    m_buttonIconSize = QSize(int(0.9 * m_baseIconHeight), int(0.9 * m_baseIconHeight));
    m_toolButtonSize = QSize(int(1.8 * m_baseIconHeight), int(1.8 * m_baseIconHeight));
}

Theme::~Theme()
{
    qDebug() << "Destructor: Theme";
}

// Getters
// =======

/**! @brief The available light themes, in display order.
 */
QList<ThemeEntry> Theme::lightThemes() const
{
    QList<ThemeEntry> result;
    for (const ThemeEntry &entry : m_themes) {
        if (!entry.dark) result.append(entry);
    }
    return result;
}

/**! @brief The available dark themes, in display order.
 */
QList<ThemeEntry> Theme::darkThemes() const
{
    QList<ThemeEntry> result;
    for (const ThemeEntry &entry : m_themes) {
        if (entry.dark) result.append(entry);
    }
    return result;
}

/**! @brief Whether a theme with the given key was found by the scan.
 */
bool Theme::hasTheme(const QString &key) const
{
    for (const ThemeEntry &entry : m_themes) {
        if (entry.key == key) return true;
    }
    return false;
}

/**! @brief Whether the desktop is in dark mode.
 *
 * The style's own palette is checked when the platform does not report a
 * colour scheme, since the application palette is overridden by the theme.
 */
bool Theme::isDesktopDarkMode() const
{
    if (QStyleHints *hints = QGuiApplication::styleHints()) {
        const Qt::ColorScheme scheme = hints->colorScheme();
        if (scheme != Qt::ColorScheme::Unknown) {
            return scheme == Qt::ColorScheme::Dark;
        }
    }
    const QPalette palette = QApplication::style()->standardPalette();
    return palette.windowText().color().lightnessF() > palette.window().color().lightnessF();
}

// Public Methods
// ==============

/**! @brief Scan a folder for theme files and record their name and mode.
 *
 * Files without a name, or with a mode other than light or dark, are
 * skipped. The default themes are listed first, then the rest by name.
 */
void Theme::scanThemes(const QDir &dir)
{
    m_themes.clear();

    const QFileInfoList files = dir.entryInfoList({"*.json"_L1}, QDir::Files | QDir::Readable, QDir::Name);
    for (const QFileInfo &file : files) {
        QJsonObject data;
        if (JsonUtils::readJson(file.absoluteFilePath(), data, true) != JsonUtilsError::NoError) continue;

        const QJsonObject jMeta = data.value("c:meta"_L1).toObject();
        const QString name = JsonUtils::getJsonString(jMeta, "m:name"_L1, "");
        const QString mode = JsonUtils::getJsonString(jMeta, "m:mode"_L1, "").toLower();
        if (name.isEmpty() || (mode != "light"_L1 && mode != "dark"_L1)) {
            qWarning() << "Skipping theme file:" << file.fileName();
            continue;
        }
        m_themes.append({file.completeBaseName(), name, mode == "dark"_L1, file.absoluteFilePath()});
    }

    std::sort(m_themes.begin(), m_themes.end(), [](const ThemeEntry &a, const ThemeEntry &b) {
        const bool aDefault = a.key.startsWith("default"_L1);
        const bool bDefault = b.key.startsWith("default"_L1);
        if (aDefault != bDefault) return aDefault;
        return a.name.localeAwareCompare(b.name) < 0;
    });

    qInfo() << "Found" << m_themes.count() << "themes";
}

/**! @brief Load the theme selected by the current settings.
 *
 * The theme mode decides whether the light or dark theme is used, following
 * the desktop when set to auto. An unknown theme falls back to the default
 * for that mode and the setting is corrected. Returns true if a theme was
 * loaded, and false if it failed or was already the current theme.
 */
bool Theme::loadTheme()
{
    bool darkMode = false;
    switch (m_settings->themeMode()) {
    case ThemeMode::LightTheme: darkMode = false; break;
    case ThemeMode::DarkTheme: darkMode = true; break;
    default: darkMode = this->isDesktopDarkMode(); break;
    }

    QString key = darkMode ? m_settings->darkTheme() : m_settings->lightTheme();
    if (!this->hasTheme(key)) {
        qWarning() << "Could not find theme:" << key;
        key = darkMode ? COL_DEFAULT_DARK_THEME : COL_DEFAULT_LIGHT_THEME;
        if (darkMode) {
            m_settings->setDarkTheme(key);
        } else {
            m_settings->setLightTheme(key);
        }
    }

    if (key == m_currentTheme) {
        qInfo() << "Theme already loaded:" << key;
        return false;
    }
    return this->loadTheme(key);
}

/**! @brief Load a theme by key and apply it to the application palette.
 */
bool Theme::loadTheme(const QString &key)
{
    QString path;
    for (const ThemeEntry &entry : std::as_const(m_themes)) {
        if (entry.key == key) {
            path = entry.path;
            break;
        }
    }
    if (path.isEmpty()) {
        qWarning() << "Unknown theme:" << key;
        return false;
    }
    qInfo() << "Loading Theme:" << key;

    QJsonObject data;
    if (JsonUtils::readJson(path, data, true) != JsonUtilsError::NoError) return false;
    m_currentTheme = key;

    QJsonObject jMeta = data.value("c:meta"_L1).toObject();
    QJsonObject jBase = data.value("c:baseColors"_L1).toObject();
    QJsonObject jTheme = data.value("c:themeColors"_L1).toObject();
    QJsonObject jIcon = data.value("c:iconColors"_L1).toObject();
    QJsonObject jSyntax = data.value("c:syntaxColors"_L1).toObject();

    // Theme Meta
    m_name = JsonUtils::getJsonString(jMeta, "m:name"_L1, "Unknown");
    m_author = JsonUtils::getJsonString(jMeta, "m:author"_L1, "");
    m_credit = JsonUtils::getJsonString(jMeta, "m:credit"_L1, "");
    m_license = JsonUtils::getJsonString(jMeta, "m:license"_L1, "");

    // Syntax Colors
    m_syntaxColors = {
        QColor::fromString(JsonUtils::getJsonString(jSyntax, "header"_L1, "blue")),        // SyntaxColor::SyntaxHeader
        QColor::fromString(JsonUtils::getJsonString(jSyntax, "emphasis"_L1, "green")),     // SyntaxColor::SyntaxEmphasis
        QColor::fromString(JsonUtils::getJsonString(jSyntax, "comment"_L1, "grey")),       // SyntaxColor::SyntaxComment
        QColor::fromString(JsonUtils::getJsonString(jSyntax, "spellCheckLine"_L1, "red")), // SyntaxColor::SyntaxSpellLine
        QColor::fromString(JsonUtils::getJsonString(jSyntax, "errorLine"_L1, "orange")),   // SyntaxColor::SyntaxErrorLine
    };

    // Qt Base Colors
    QColor window = QColor::fromString(JsonUtils::getJsonString(jBase, "window"_L1, "white"));
    QColor windowText = QColor::fromString(JsonUtils::getJsonString(jBase, "windowText"_L1, "black"));
    QColor base = QColor::fromString(JsonUtils::getJsonString(jBase, "base"_L1, "white"));
    QColor alternateBase = QColor::fromString(JsonUtils::getJsonString(jBase, "alternateBase"_L1, "grey"));
    QColor text = QColor::fromString(JsonUtils::getJsonString(jBase, "text"_L1, "black"));
    QColor toolTipBase = QColor::fromString(JsonUtils::getJsonString(jBase, "toolTipBase"_L1, "yellow"));
    QColor toolTipText = QColor::fromString(JsonUtils::getJsonString(jBase, "toolTipText"_L1, "black"));
    QColor button = QColor::fromString(JsonUtils::getJsonString(jBase, "button"_L1, "white"));
    QColor buttonText = QColor::fromString(JsonUtils::getJsonString(jBase, "buttonText"_L1, "black"));
    QColor brightText = QColor::fromString(JsonUtils::getJsonString(jBase, "brightText"_L1, "black"));
    QColor highlight = QColor::fromString(JsonUtils::getJsonString(jBase, "highlight"_L1, "red"));
    QColor highlightedText = QColor::fromString(JsonUtils::getJsonString(jBase, "highlightedText"_L1, "grey"));
    QColor link = QColor::fromString(JsonUtils::getJsonString(jBase, "link"_L1, "blue"));
    QColor linkVisited = QColor::fromString(JsonUtils::getJsonString(jBase, "linkVisited"_L1, "purple"));

    // The accent colour falls back to the highlight colour
    QColor accent = QColor::fromString(JsonUtils::getJsonString(jBase, "accent"_L1, ""));
    m_accentColor = accent.isValid() ? accent : highlight;

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

    // The help text colour falls back to the faded colour
    QColor helpText = QColor::fromString(JsonUtils::getJsonString(jTheme, "helpText"_L1, ""));
    m_helpTextColor = helpText.isValid() ? helpText : m_colors.at(ThemeColor::FadedColor);

    // Icon Colors
    // Each category falls back to one of the theme colours above.
    auto iconColor = [&](QLatin1StringView key, ThemeColor fallback) -> QColor {
        QColor color = QColor::fromString(JsonUtils::getJsonString(jIcon, key, ""));
        return color.isValid() ? color : m_colors.at(fallback);
    };
    m_colors.append({
        iconColor("tool"_L1, ThemeColor::DefaultColor),  // ThemeColor::ToolColor
        iconColor("accept"_L1, ThemeColor::Green),       // ThemeColor::AcceptColor
        iconColor("reject"_L1, ThemeColor::Red),         // ThemeColor::RejectColor
        iconColor("action"_L1, ThemeColor::Blue),        // ThemeColor::ActionColor
        iconColor("option"_L1, ThemeColor::Orange),      // ThemeColor::OptionColor
        iconColor("apply"_L1, ThemeColor::Green),        // ThemeColor::ApplyColor
        iconColor("create"_L1, ThemeColor::Yellow),      // ThemeColor::CreateColor
        iconColor("destroy"_L1, ThemeColor::FadedColor), // ThemeColor::DestroyColor
        iconColor("reset"_L1, ThemeColor::Green),        // ThemeColor::ResetColor
        iconColor("add"_L1, ThemeColor::Green),          // ThemeColor::AddColor
        iconColor("change"_L1, ThemeColor::Green),       // ThemeColor::ChangeColor
        iconColor("remove"_L1, ThemeColor::Red),         // ThemeColor::RemoveColor
    });

    // Cached icons were rendered with the previous colours
    m_icons->clearCache();

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

    QColor light = ref.lighter(150);
    QColor mid = ref.darker(130);
    QColor midLight = mid.lighter(110);
    QColor dark = ref.darker(150);
    QColor shadow = dark.darker(135);
    QColor darkOff = dark.darker(150);
    QColor shadowOff = ref.darker(150);

    QColor grey = m_isDark ? QColor(120, 120, 120) : QColor(140, 140, 140);
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

    palette.setBrush(QPalette::Active, QPalette::Highlight, highlight);
    palette.setBrush(QPalette::Inactive, QPalette::Highlight, highlight);
    palette.setBrush(QPalette::Disabled, QPalette::Highlight, grey);

    palette.setBrush(QPalette::Active, QPalette::Accent, m_accentColor);
    palette.setBrush(QPalette::Inactive, QPalette::Accent, m_accentColor);
    palette.setBrush(QPalette::Disabled, QPalette::Accent, grey);

    QApplication::setPalette(palette);

    emit themeChanged();

    return true;
}

// Private Slots
// =============

/**! @brief Reload the theme when the desktop switches between light and dark.
 *
 * Only applies when the theme mode follows the desktop.
 */
void Theme::onColorSchemeChanged()
{
    if (m_settings->themeMode() == ThemeMode::AutoTheme) {
        this->loadTheme();
    }
}

/**! @brief Create an icon-only tool button with the standard tool tip and icon.
 *
 * The caller owns the button through its parent.
 */
MIconButton *Theme::getIconButton(ToolButton button, QWidget *parent) const
{
    const StandardButtonDef &def = toolButtons[button];
    MIconButton *iconButton = new MIconButton(parent);
    iconButton->setThemeIcon(def.icon, def.color);
    iconButton->setToolTip(QCoreApplication::translate("Button", def.text));
    return iconButton;
}

/**! @brief Create a push button with the standard label and icon.
 *
 * The caller owns the button through its parent.
 */
MPushButton *Theme::getStandardButton(StandardButton button, QWidget *parent) const
{
    const StandardButtonDef &def = standardButtons[button];
    MPushButton *pushButton = new MPushButton(QCoreApplication::translate("Button", def.text), parent);
    pushButton->setThemeIcon(def.icon, def.color);
    return pushButton;
}

} // namespace Collett
