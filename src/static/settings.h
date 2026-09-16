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

#include <QDir>
#include <QFont>
#include <QList>
#include <QScopedPointer>
#include <QSize>
#include <QString>
#include <QTextBlockFormat>
#include <QTextCharFormat>

namespace Collett {

class Settings : public QObject
{
    Q_OBJECT

public:
    struct TextFormat
    {
        QTextBlockFormat blockDefault;
        QTextCharFormat charDefault;
        QTextBlockFormat blockParagraph;
        QTextCharFormat charParagraph;
        QTextBlockFormat blockComment;
        QTextCharFormat charComment;
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

    explicit Settings(QObject *parent = nullptr);
    ~Settings() noexcept;

    // Methods
    void flushSettings();
    static QDir assetPath(QString asset);

    // Setters
    void setMainWindowSize(const QSize size) { m_mainWindowSize = size; };
    void setMainSplitSizes(const QList<int> &sizes) { m_mainSplitSizes = sizes; };
    void setPrefsWindowSize(const QSize size) { m_prefsWindowSize = size; };
    void setThemeMode(const ThemeMode mode) { m_themeMode = mode; };
    void setLightTheme(const QString &theme) { m_lightTheme = theme; };
    void setDarkTheme(const QString &theme) { m_darkTheme = theme; };
    void setMainIconSet(const QString icons) { m_iconSet = icons; };
    void setGuiFont(const QFont &font) { m_guiFont = font; };
    void setNativeFontDialog(const bool native) { m_nativeFontDialog = native; };
    void setEditorAutoSave(const int interval) { m_editorAutoSave = interval; };
    void setSpellLanguage(const QString &language) { m_spellLanguage = language.trimmed(); };
    void setTextFont(const QFont &font);
    void setMonoFont(const QFont &font) { m_monoFont = font; };
    void setTextTabWidth(const qreal width);

    // Getters
    QSize mainWindowSize() const { return m_mainWindowSize; };
    QList<int> mainSplitSizes() const { return m_mainSplitSizes; };
    QSize prefsWindowSize() const { return m_prefsWindowSize; };
    ThemeMode themeMode() const { return m_themeMode; };
    QString lightTheme() const { return m_lightTheme; };
    QString darkTheme() const { return m_darkTheme; };
    QString iconSet() const { return m_iconSet; };
    QFont guiFont() const { return m_guiFont; };
    bool nativeFontDialog() const { return m_nativeFontDialog; };
    int editorAutoSave() const { return m_editorAutoSave; };
    QString spellLanguage() const { return m_spellLanguage; };
    QFont textFont() const { return m_textFont; };
    QFont monoFont() const { return m_monoFont; };
    TextFormat textFormat() const { return m_textFormat; };

private:
    static Settings *staticInstance;

    // GUI Settings
    QSize m_mainWindowSize;
    QList<int> m_mainSplitSizes;
    QSize m_prefsWindowSize;
    ThemeMode m_themeMode;
    QString m_lightTheme;
    QString m_darkTheme;
    QString m_iconSet;
    QFont m_guiFont;
    bool m_nativeFontDialog;

    // Editor
    int m_editorAutoSave;

    // Spell Check
    QString m_spellLanguage;

    // Text Format
    QFont m_textFont;
    QFont m_monoFont;
    qreal m_textFontSize;
    qreal m_textTabWidth;
    TextFormat m_textFormat;

    // Internal Functions
    void recalculateTextFormats();
};
} // namespace Collett
