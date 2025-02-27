/*
** Collett – Main Settings Class
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

#ifndef COLLETT_SETTINGS_H
#define COLLETT_SETTINGS_H

#include "collett.h"

#include <QDir>
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
    struct TextFormat {
        QTextBlockFormat blockDefault;
        QTextCharFormat  charDefault;
        QTextBlockFormat blockParagraph;
        QTextCharFormat  charParagraph;
        QTextBlockFormat blockHeader1;
        QTextCharFormat  charHeader1;
        QTextBlockFormat blockHeader2;
        QTextCharFormat  charHeader2;
        QTextBlockFormat blockHeader3;
        QTextCharFormat  charHeader3;
        QTextBlockFormat blockHeader4;
        QTextCharFormat  charHeader4;
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

    void setMainWindowSize(const QSize size) {m_mainWindowSize = size;};
    void setMainSplitSizes(const QList<int> &sizes) {m_mainSplitSizes = sizes;};
    void setMainGuiTheme(const QString theme) {m_guiTheme = theme;};
    void setEditorAutoSave(const int interval) {m_editorAutoSave = interval;};
    void setTextFontSize(const qreal size);
    void setTextTabWidth(const qreal width);

    // Getters

    QSize      mainWindowSize() const {return m_mainWindowSize;};
    QList<int> mainSplitSizes() const {return m_mainSplitSizes;};
    QString    guiTheme() const {return m_guiTheme;};
    int        editorAutoSave() const {return m_editorAutoSave;};
    TextFormat textFormat() const {return m_textFormat;};

private:
    static Settings *staticInstance;

    // GUI Settings

    QSize      m_mainWindowSize;
    QList<int> m_mainSplitSizes;
    QString    m_guiTheme;

    // Editor

    int m_editorAutoSave;

    // Text Format

    qreal      m_textFontSize;
    qreal      m_textTabWidth;
    TextFormat m_textFormat;

    // Internal Functions

    void recalculateTextFormats();

};
} // namespace Collett

#endif // COLLETT_SETTINGS_H
