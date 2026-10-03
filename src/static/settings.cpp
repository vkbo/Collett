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
#include "spellchecker.h"

#include <QFont>
#include <QDir>
#include <QFontDatabase>
#include <QJSEngine>
#include <QLocale>
#include <QSettings>
#include <QTextBlockFormat>
#include <QTextCharFormat>

using namespace Qt::Literals::StringLiterals;

#define CNF_GUI_LANGUAGE "Main/guiLanguage"_L1
#define CNF_PREFS_WINDOW_SIZE "Main/prefsWindowSize"_L1
#define CNF_EDITOR_AUTO_SAVE "Editor/autoSave"_L1
#define CNF_TEXT_FONT "TextFormat/textFont"_L1
#define CNF_TEXT_TAB_WIDTH "TextFormat/tabWidth"_L1
#define CNF_SPELL_LANGUAGE "SpellCheck/language"_L1

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

/**! @brief The instance for QML, which C++ keeps ownership of.
 */
Settings *Settings::create(QQmlEngine *, QJSEngine *)
{
    Settings *settings = instance();
    QJSEngine::setObjectOwnership(settings, QJSEngine::CppOwnership);
    return settings;
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

    // GUI Settings
    // ------------

    m_guiLanguage = settings.value(CNF_GUI_LANGUAGE, QLocale::system().name()).toString();
    m_prefsWindowSize = settings.value(CNF_PREFS_WINDOW_SIZE, QSize(900, 700)).toSize();

    // Editor Settings
    // ---------------

    m_editorAutoSave = qMax(settings.value(CNF_EDITOR_AUTO_SAVE, 30).toInt(), 5);

    // Text Format
    // -----------

    QFont defaultTextFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    defaultTextFont.setPointSizeF(13.0);
    m_textFont = fontFromSettings(settings, CNF_TEXT_FONT, defaultTextFont);
    m_textFontSize = qMax(m_textFont.pointSizeF(), 5.0);
    m_textTabWidth = qMax(settings.value(CNF_TEXT_TAB_WIDTH, (qreal)40.0).toReal(), 0.0);
    recalculateTextFormats();

    // Spell Check
    // -----------

    // The system language is the default. If no dictionary is installed for
    // it, the spell checker falls back to accepting all words.
    m_spellLanguage = settings.value(CNF_SPELL_LANGUAGE, QLocale::system().name()).toString();
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

    settings.setValue(CNF_GUI_LANGUAGE, m_guiLanguage);
    settings.setValue(CNF_PREFS_WINDOW_SIZE, m_prefsWindowSize);
    settings.setValue(CNF_EDITOR_AUTO_SAVE, m_editorAutoSave);

    settings.setValue(CNF_TEXT_FONT, m_textFont.toString());
    settings.setValue(CNF_TEXT_TAB_WIDTH, m_textTabWidth);
    settings.setValue(CNF_SPELL_LANGUAGE, m_spellLanguage);

    qDebug() << "Settings values saved";

    return;
}

/**! @brief The languages the GUI can be shown in, as value and text pairs.
 *
 * The source text is British English, and the others are the translations
 * built into the app.
 */
QVariantList Settings::guiLanguages() const
{
    QStringList tags = {u"en_GB"_s};
    const QStringList files = QDir(u":/i18n"_s).entryList({u"collett_*.qm"_s}, QDir::Files);
    for (const QString &file : files) {
        tags.append(file.sliced(8).chopped(3));
    }

    QVariantList languages;
    for (const QString &tag : std::as_const(tags)) {
        QString name = QLocale(tag).nativeLanguageName();
        if (!name.isEmpty()) name[0] = name[0].toUpper();
        languages.append(QVariantMap{{u"value"_s, tag}, {u"text"_s, name.isEmpty() ? tag : name}});
    }
    return languages;
}

/**! @brief The installed spell checking dictionaries, as value and text
 * pairs.
 */
QVariantList Settings::spellLanguages() const
{
    QVariantList languages;
    const SpellChecker checker;
    for (const SpellChecker::Language &language : checker.listDictionaries()) {
        languages.append(QVariantMap{{u"value"_s, language.tag}, {u"text"_s, language.name}});
    }
    return languages;
}

// Setters
// =======

void Settings::setGuiLanguage(const QString &language)
{
    if (language.trimmed() == m_guiLanguage) return;
    m_guiLanguage = language.trimmed();
    emit guiLanguageChanged();
}

void Settings::setPrefsWindowSize(const QSize &size)
{
    if (size == m_prefsWindowSize) return;
    m_prefsWindowSize = size;
    emit prefsWindowSizeChanged();
}

void Settings::setEditorAutoSave(const int interval)
{
    if (qMax(interval, 5) == m_editorAutoSave) return;
    m_editorAutoSave = qMax(interval, 5);
    emit editorAutoSaveChanged();
}

void Settings::setSpellLanguage(const QString &language)
{
    if (language.trimmed() == m_spellLanguage) return;
    m_spellLanguage = language.trimmed();
    emit spellLanguageChanged();
}

/**! @brief Set the document font and rebuild the text formats from it.
 */
void Settings::setTextFont(const QFont &font)
{
    if (font == m_textFont) return;
    m_textFont = font;
    m_textFontSize = qMax(font.pointSizeF(), 5.0);
    recalculateTextFormats();
    emit textFormatChanged();
}

void Settings::setTextTabWidth(const qreal width)
{
    if (width == m_textTabWidth) return;
    m_textTabWidth = width;
    recalculateTextFormats();
    emit textFormatChanged();
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
