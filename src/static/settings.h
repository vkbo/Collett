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

    // Setters
    void setGuiLanguage(const QString &language) { m_guiLanguage = language.trimmed(); };
    void setEditorAutoSave(const int interval) { m_editorAutoSave = interval; };
    void setTextFont(const QFont &font);
    void setTextTabWidth(const qreal width);
    void setSpellLanguage(const QString &language) { m_spellLanguage = language.trimmed(); };

    // Getters
    QString guiLanguage() const { return m_guiLanguage; };
    int editorAutoSave() const { return m_editorAutoSave; };
    QFont textFont() const { return m_textFont; };
    TextFormat textFormat() const { return m_textFormat; };
    QString spellLanguage() const { return m_spellLanguage; };

signals:
    void textFormatChanged();

private:
    static Settings *staticInstance;

    // GUI
    QString m_guiLanguage;

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
