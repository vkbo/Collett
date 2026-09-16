/*
** Collett - Main Settings Class
** =============================
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
#include "settings.h"

#include <QCoreApplication>
#include <QDir>
#include <QFont>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QList>
#include <QLocale>
#include <QSettings>
#include <QSize>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QVariant>
#include <QVariantList>

using namespace Qt::Literals::StringLiterals;

#define CNF_EDITOR_AUTO_SAVE "Editor/autoSave"_L1
#define CNF_MAIN_SPLIT_SIZES "Main/mainSplitSizes"_L1
#define CNF_MAIN_WINDOW_SIZE "Main/windowSize"_L1
#define CNF_MAIN_THEME_MODE "Main/themeMode"_L1
#define CNF_MAIN_LIGHT_THEME "Main/lightTheme"_L1
#define CNF_MAIN_DARK_THEME "Main/darkTheme"_L1
#define CNF_MAIN_ICON_SET "Main/iconSet"_L1
#define CNF_MAIN_GUI_FONT "Main/guiFont"_L1
#define CNF_MAIN_NATIVE_FONT_DIALOG "Main/nativeFontDialog"_L1
#define CNF_MAIN_PREFS_WINDOW_SIZE "Main/prefsWindowSize"_L1
#define CNF_SPELL_LANGUAGE "SpellCheck/language"_L1
#define CNF_TEXT_FONT "TextFormat/textFont"_L1
#define CNF_TEXT_MONO_FONT "TextFormat/monoFont"_L1
#define CNF_TEXT_TAB_WIDTH "TextFormat/tabWidth"_L1

namespace Collett {

// Converter Functions
// ===================

/**! @brief Read a font from settings, falling back to a default.
 *
 * Fonts are stored in the string form used by QFont, so a missing or
 * unreadable value returns the fallback untouched.
 */
QFont fontFromSettings(const QSettings &settings, const QLatin1StringView key, const QFont &fallback)
{
    QFont font;
    const QString value = settings.value(key, QString()).toString();
    if (!value.isEmpty() && font.fromString(value)) {
        return font;
    }
    return fallback;
}

QList<int> variantListToInt(const QVariantList &list)
{
    QList<int> result;
    for (const QVariant &val : list) {
        result.append(val.toInt());
    }
    return result;
}

QVariantList intListToVariant(const QList<int> &list)
{
    QVariantList result;
    for (const int &val : list) {
        result.append(val);
    }
    return result;
}

// Constructor/Destructor/Instance
// ===============================

Settings *Settings::staticInstance = nullptr;
Settings *Settings::instance()
{
    if (staticInstance == nullptr) {
        staticInstance = new Settings();
        qDebug() << "Constructor: Settings";
    }
    return staticInstance;
}

void Settings::destroy()
{
    if (staticInstance != nullptr) {
        qDebug() << "Destructor: Static Settings";
        delete Settings::staticInstance;
        Settings::staticInstance = nullptr;
    }
}

Settings::Settings(QObject *parent) : QObject(parent)
{

    // Load Settings
    QSettings settings;

    // Main Settings
    // -------------

    m_mainWindowSize = settings.value(CNF_MAIN_WINDOW_SIZE, QSize(1200, 800)).toSize();
    m_mainSplitSizes = variantListToInt(settings.value(CNF_MAIN_SPLIT_SIZES, QVariantList() << 300 << 700).toList());
    m_prefsWindowSize = settings.value(CNF_MAIN_PREFS_WINDOW_SIZE, QSize(700, 615)).toSize();
    m_themeMode = ThemeMode(qBound(int(ThemeMode::AutoTheme), settings.value(CNF_MAIN_THEME_MODE, int(ThemeMode::AutoTheme)).toInt(), int(ThemeMode::DarkTheme)));
    m_lightTheme = settings.value(CNF_MAIN_LIGHT_THEME, COL_DEFAULT_LIGHT_THEME).toString();
    m_darkTheme = settings.value(CNF_MAIN_DARK_THEME, COL_DEFAULT_DARK_THEME).toString();
    m_iconSet = settings.value(CNF_MAIN_ICON_SET, "lucide").toString();
    m_guiFont = fontFromSettings(settings, CNF_MAIN_GUI_FONT, QGuiApplication::font());
    m_nativeFontDialog = settings.value(CNF_MAIN_NATIVE_FONT_DIALOG, true).toBool();

    // Check Values
    if (m_mainWindowSize.width() < 400) m_mainWindowSize.setWidth(400);
    if (m_mainWindowSize.height() < 300) m_mainWindowSize.setHeight(300);
    if (m_prefsWindowSize.width() < 600) m_prefsWindowSize.setWidth(600);
    if (m_prefsWindowSize.height() < 500) m_prefsWindowSize.setHeight(500);

    // Editor Settings
    // ---------------

    m_editorAutoSave = qMax(settings.value(CNF_EDITOR_AUTO_SAVE, 30).toInt(), 5);

    // Spell Check
    // -----------

    // The default is the system locale, like "en_GB". If no dictionary exists
    // for it, the spell checker falls back to accepting all words.
    m_spellLanguage = settings.value(CNF_SPELL_LANGUAGE, QLocale::system().name()).toString();

    // Text Format
    // -----------

    QFont defaultTextFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    defaultTextFont.setPointSizeF(13.0);
    m_textFont = fontFromSettings(settings, CNF_TEXT_FONT, defaultTextFont);
    m_monoFont = fontFromSettings(settings, CNF_TEXT_MONO_FONT, QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_textFontSize = qMax(m_textFont.pointSizeF(), 5.0);
    m_textTabWidth = qMax(settings.value(CNF_TEXT_TAB_WIDTH, (qreal)40.0).toReal(), 0.0);
    recalculateTextFormats();
}

Settings::~Settings()
{
    qDebug() << "Destructor: Settings";
}

// Public Methods
// ==============

void Settings::flushSettings()
{

    QSettings settings;

    settings.setValue(CNF_MAIN_WINDOW_SIZE, m_mainWindowSize);
    settings.setValue(CNF_MAIN_SPLIT_SIZES, intListToVariant(m_mainSplitSizes));
    settings.setValue(CNF_MAIN_PREFS_WINDOW_SIZE, m_prefsWindowSize);
    settings.setValue(CNF_MAIN_THEME_MODE, int(m_themeMode));
    settings.setValue(CNF_MAIN_LIGHT_THEME, m_lightTheme);
    settings.setValue(CNF_MAIN_DARK_THEME, m_darkTheme);
    settings.setValue(CNF_MAIN_ICON_SET, m_iconSet);
    settings.setValue(CNF_MAIN_GUI_FONT, m_guiFont.toString());
    settings.setValue(CNF_MAIN_NATIVE_FONT_DIALOG, m_nativeFontDialog);

    settings.setValue(CNF_EDITOR_AUTO_SAVE, m_editorAutoSave);

    settings.setValue(CNF_SPELL_LANGUAGE, m_spellLanguage);

    settings.setValue(CNF_TEXT_FONT, m_textFont.toString());
    settings.setValue(CNF_TEXT_MONO_FONT, m_monoFont.toString());
    settings.setValue(CNF_TEXT_TAB_WIDTH, m_textTabWidth);

    qDebug() << "Settings values saved";

    return;
}

QDir Settings::assetPath(QString asset)
{
    return QDir(QCoreApplication::applicationDirPath() + "/assets/" + asset);
}

// Setters
// =======

/**! @brief Set the document font and rebuild the text formats from it.
 */
void Settings::setTextFont(const QFont &font)
{
    m_textFont = font;
    m_textFontSize = qMax(font.pointSizeF(), 5.0);
    recalculateTextFormats();
}

void Settings::setTextTabWidth(const qreal width)
{
    m_textTabWidth = width;
    recalculateTextFormats();
}

// Internal Functions
// ==================

void Settings::recalculateTextFormats()
{

    // Text Formats

    QTextCharFormat defaultCharFmt;
    QTextBlockFormat defaultBlockFmt;

    // Default Values

    qreal defaultLineHeight = 1.15;
    qreal defaultTopMargin = 0.5 * m_textFontSize;
    qreal defaultBottomMargin = 0.5 * m_textFontSize;

    qreal header1FontSize = 2.0 * m_textFontSize;
    qreal header2FontSize = 1.7 * m_textFontSize;
    qreal header3FontSize = 1.4 * m_textFontSize;
    qreal header4FontSize = 1.2 * m_textFontSize;

    qreal headerBottomMargin = 0.7 * m_textFontSize;

    // Text Format Values

    m_textFormat.fontSize = m_textFontSize;
    m_textFormat.tabWidth = m_textTabWidth;
    m_textFormat.lineHeight = 1.15;

    // Default Text Formats

    defaultBlockFmt.setHeadingLevel(0);
    defaultBlockFmt.setLineHeight(defaultLineHeight, QTextBlockFormat::SingleHeight);
    defaultBlockFmt.setTopMargin(defaultTopMargin);
    defaultBlockFmt.setBottomMargin(defaultBottomMargin);
    defaultBlockFmt.setTextIndent(0.0);
    m_textFormat.blockDefault = defaultBlockFmt;

    defaultCharFmt.setFontFamilies({m_textFont.family()});
    defaultCharFmt.setFontPointSize(m_textFontSize);
    m_textFormat.charDefault = defaultCharFmt;

    // Paragraph Formats

    m_textFormat.blockParagraph = defaultBlockFmt;
    m_textFormat.charParagraph = defaultCharFmt;

    // Comment Formats
    // A comment is a paragraph flagged by a block property. Its colouring is
    // handled by the editor's highlighter, not by the stored formats.

    m_textFormat.blockComment = defaultBlockFmt;
    m_textFormat.blockComment.setProperty(BlockTypeProperty, CommentBlock);
    m_textFormat.charComment = defaultCharFmt;

    // Header 1 Formats

    m_textFormat.blockHeader1 = defaultBlockFmt;
    m_textFormat.blockHeader1.setHeadingLevel(1);
    m_textFormat.blockHeader1.setTopMargin(header1FontSize);
    m_textFormat.blockHeader1.setBottomMargin(headerBottomMargin);

    m_textFormat.charHeader1 = defaultCharFmt;
    m_textFormat.charHeader1.setFontPointSize(header1FontSize);
    m_textFormat.charHeader1.setFontWeight(QFont::Bold);

    // Header 2 Formats

    m_textFormat.blockHeader2 = defaultBlockFmt;
    m_textFormat.blockHeader2.setHeadingLevel(2);
    m_textFormat.blockHeader2.setTopMargin(header2FontSize);
    m_textFormat.blockHeader2.setBottomMargin(headerBottomMargin);

    m_textFormat.charHeader2 = defaultCharFmt;
    m_textFormat.charHeader2.setFontPointSize(header2FontSize);
    m_textFormat.charHeader2.setFontWeight(QFont::Bold);

    // Header 3 Formats

    m_textFormat.blockHeader3 = defaultBlockFmt;
    m_textFormat.blockHeader3.setHeadingLevel(3);
    m_textFormat.blockHeader3.setTopMargin(header3FontSize);
    m_textFormat.blockHeader3.setBottomMargin(headerBottomMargin);

    m_textFormat.charHeader3 = defaultCharFmt;
    m_textFormat.charHeader3.setFontPointSize(header3FontSize);
    m_textFormat.charHeader3.setFontWeight(QFont::Bold);

    // Header 4 Formats

    m_textFormat.blockHeader4 = defaultBlockFmt;
    m_textFormat.blockHeader4.setHeadingLevel(4);
    m_textFormat.blockHeader4.setTopMargin(header4FontSize);
    m_textFormat.blockHeader4.setBottomMargin(headerBottomMargin);

    m_textFormat.charHeader4 = defaultCharFmt;
    m_textFormat.charHeader4.setFontPointSize(header4FontSize);
    m_textFormat.charHeader4.setFontWeight(QFont::Bold);
}

} // namespace Collett
