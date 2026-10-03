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

#pragma once

#include "collett.h"

#include <QFont>
#include <QObject>
#include <QSize>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

class QQmlEngine;
class QJSEngine;
#include <QTextBlockFormat>
#include <QTextCharFormat>

namespace Collett {

/**! @brief The application settings, shared by C++ and QML.
 *
 * QML gets the same instance as C++, as a singleton.
 */
class Settings : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString guiLanguage READ guiLanguage WRITE setGuiLanguage NOTIFY guiLanguageChanged)
    Q_PROPERTY(ThemeMode themeMode READ themeMode WRITE setThemeMode NOTIFY themeModeChanged)
    Q_PROPERTY(QSize prefsWindowSize READ prefsWindowSize WRITE setPrefsWindowSize NOTIFY prefsWindowSizeChanged)
    Q_PROPERTY(int sideBarWidth READ sideBarWidth WRITE setSideBarWidth NOTIFY sideBarWidthChanged)
    Q_PROPERTY(int editorAutoSave READ editorAutoSave WRITE setEditorAutoSave NOTIFY editorAutoSaveChanged)
    Q_PROPERTY(QFont guiFont READ guiFont WRITE setGuiFont NOTIFY guiFontChanged)
    Q_PROPERTY(QFont textFont READ textFont WRITE setTextFont NOTIFY textFormatChanged)
    Q_PROPERTY(QFont headingFont READ headingFont WRITE setHeadingFont NOTIFY textFormatChanged)
    Q_PROPERTY(QFont monoFont READ monoFont WRITE setMonoFont NOTIFY monoFontChanged)
    Q_PROPERTY(qreal textTabWidth READ textTabWidth WRITE setTextTabWidth NOTIFY textFormatChanged)
    Q_PROPERTY(bool textAutoIndent READ textAutoIndent WRITE setTextAutoIndent NOTIFY textAutoIndentChanged)
    Q_PROPERTY(bool showMultiSpaces READ showMultiSpaces WRITE setShowMultiSpaces NOTIFY showMultiSpacesChanged)
    Q_PROPERTY(QString spellLanguage READ spellLanguage WRITE setSpellLanguage NOTIFY spellLanguageChanged)

public:
    // Whether the colours follow the system's light or dark mode
    enum ThemeMode
    {
        AutoTheme = 0,
        LightTheme = 1,
        DarkTheme = 2,
    };
    Q_ENUM(ThemeMode)

    struct TextFormat
    {
        QTextBlockFormat blockDefault;
        QTextCharFormat charDefault;
        QTextBlockFormat blockParagraph;
        QTextCharFormat charParagraph;
        QTextBlockFormat blockHeader1;
        QTextCharFormat charHeader1;
        QTextBlockFormat blockHeader2;
        QTextCharFormat charHeader2;
        QTextBlockFormat blockHeader3;
        QTextCharFormat charHeader3;
        QTextBlockFormat blockHeader4;
        QTextCharFormat charHeader4;
        qreal fontSize;
        qreal tabWidth;
        qreal lineHeight;
    };

    static Settings *instance();
    static void destroy();
    static Settings *create(QQmlEngine *, QJSEngine *);

    // Methods
    Q_INVOKABLE void flushSettings();
    Q_INVOKABLE QVariantList guiLanguages() const;
    Q_INVOKABLE QVariantList spellLanguages() const;

    // Setters
    void setGuiLanguage(const QString &language);
    void setThemeMode(const ThemeMode mode);
    void setPrefsWindowSize(const QSize &size);
    void setSideBarWidth(const int width);
    void setEditorAutoSave(const int interval);
    void setGuiFont(const QFont &font);
    void setTextFont(const QFont &font);
    void setHeadingFont(const QFont &font);
    void setMonoFont(const QFont &font);
    void setTextTabWidth(const qreal width);
    void setTextAutoIndent(const bool enabled);
    void setShowMultiSpaces(const bool enabled);
    void setSpellLanguage(const QString &language);

    // Getters
    QString guiLanguage() const { return m_guiLanguage; };
    ThemeMode themeMode() const { return m_themeMode; };
    QSize prefsWindowSize() const { return m_prefsWindowSize; };
    int sideBarWidth() const { return m_sideBarWidth; };
    int editorAutoSave() const { return m_editorAutoSave; };
    QFont guiFont() const { return m_guiFont; };
    QFont textFont() const { return m_textFont; };
    QFont headingFont() const { return m_headingFont; };
    QFont monoFont() const { return m_monoFont; };
    qreal textTabWidth() const { return m_textTabWidth; };
    bool textAutoIndent() const { return m_textAutoIndent; };
    bool showMultiSpaces() const { return m_showMultiSpaces; };
    TextFormat textFormat() const { return m_textFormat; };
    QString spellLanguage() const { return m_spellLanguage; };

signals:
    void guiLanguageChanged();
    void themeModeChanged();
    void prefsWindowSizeChanged();
    void sideBarWidthChanged();
    void editorAutoSaveChanged();
    void textAutoIndentChanged();
    void showMultiSpacesChanged();
    void guiFontChanged();
    void textFormatChanged();
    void monoFontChanged();
    void spellLanguageChanged();

private:
    // Private, so QML uses create() and shares the instance
    explicit Settings(QObject *parent = nullptr);
    ~Settings() noexcept;

    static Settings *staticInstance;

    // GUI
    QString m_guiLanguage;
    ThemeMode m_themeMode;
    QSize m_prefsWindowSize;
    int m_sideBarWidth;

    // Editor
    int m_editorAutoSave;

    // Fonts
    QFont m_guiFont;
    QFont m_textFont;
    QFont m_headingFont;
    QFont m_monoFont;

    // Text Format
    qreal m_textFontSize;
    qreal m_textTabWidth;
    bool m_textAutoIndent;
    bool m_showMultiSpaces;
    TextFormat m_textFormat;

    // Spell Check
    QString m_spellLanguage;

    // Internal Functions
    void recalculateTextFormats();
};
} // namespace Collett
