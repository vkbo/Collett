/*
** Collett – Main Settings Class
** =============================
**
** This file is a part of Collett
** Copyright 2021–2022, Veronica Berglyd Olsen
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

#include <QList>
#include <QSize>
#include <QObject>
#include <QScopedPointer>
#include <QTextCharFormat>
#include <QTextBlockFormat>

namespace Collett {

class CollettSettings : public QObject
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

    static CollettSettings *instance();
    static void destroy();

    explicit CollettSettings();
    ~CollettSettings();

    void flushSettings();

    // Setters

    void setMainWindowSize(const QSize size);
    void setMainSplitSizes(const QList<int> &sizes);
    void setEditorAutoSave(const int interval);
    void setTextFontSize(const qreal size);
    void setTextTabWidth(const qreal width);

    // Getters

    QSize      mainWindowSize() const;
    QList<int> mainSplitSizes() const;
    int        editorAutoSave() const;
    TextFormat textFormat() const;

private:
    static CollettSettings *staticInstance;

    // GUI Settings

    QSize      m_mainWindowSize;
    QList<int> m_mainSplitSizes;

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
