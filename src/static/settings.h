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
    Q_PROPERTY(QSize prefsWindowSize READ prefsWindowSize WRITE setPrefsWindowSize NOTIFY prefsWindowSizeChanged)
    Q_PROPERTY(int editorAutoSave READ editorAutoSave WRITE setEditorAutoSave NOTIFY editorAutoSaveChanged)
    Q_PROPERTY(QFont textFont READ textFont WRITE setTextFont NOTIFY textFormatChanged)
    Q_PROPERTY(qreal textTabWidth READ textTabWidth WRITE setTextTabWidth NOTIFY textFormatChanged)
    Q_PROPERTY(bool nativeFontDialog READ nativeFontDialog WRITE setNativeFontDialog NOTIFY nativeFontDialogChanged)
    Q_PROPERTY(QString spellLanguage READ spellLanguage WRITE setSpellLanguage NOTIFY spellLanguageChanged)

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
    static Settings *create(QQmlEngine *, QJSEngine *);

    // Methods
    Q_INVOKABLE void flushSettings();
    Q_INVOKABLE QVariantList guiLanguages() const;
    Q_INVOKABLE QVariantList spellLanguages() const;
    Q_INVOKABLE static QString fontDescription(const QFont &font);

    // Setters
    void setGuiLanguage(const QString &language);
    void setPrefsWindowSize(const QSize &size);
    void setEditorAutoSave(const int interval);
    void setTextFont(const QFont &font);
    void setTextTabWidth(const qreal width);
    void setNativeFontDialog(const bool state);
    void setSpellLanguage(const QString &language);

    // Getters
    QString guiLanguage() const { return m_guiLanguage; };
    QSize prefsWindowSize() const { return m_prefsWindowSize; };
    int editorAutoSave() const { return m_editorAutoSave; };
    QFont textFont() const { return m_textFont; };
    qreal textTabWidth() const { return m_textTabWidth; };
    TextFormat textFormat() const { return m_textFormat; };
    bool nativeFontDialog() const { return m_nativeFontDialog; };
    QString spellLanguage() const { return m_spellLanguage; };

signals:
    void guiLanguageChanged();
    void prefsWindowSizeChanged();
    void editorAutoSaveChanged();
    void textFormatChanged();
    void nativeFontDialogChanged();
    void spellLanguageChanged();

private:
    // Private, so QML uses create() and shares the instance
    explicit Settings(QObject *parent = nullptr);
    ~Settings() noexcept;

    static Settings *staticInstance;

    // GUI
    QString m_guiLanguage;
    QSize m_prefsWindowSize;
    bool m_nativeFontDialog;

    // Editor
    int m_editorAutoSave;

    // Text Format
    QFont m_textFont;
    qreal m_textFontSize;
    qreal m_textTabWidth;
    TextFormat m_textFormat;

    // Spell Check
    QString m_spellLanguage;

    // Internal Functions
    void recalculateTextFormats();
};
} // namespace Collett
