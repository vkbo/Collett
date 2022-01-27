/*
** Collett – Main Settings Class
** =============================
**
** This file is a part of Collett
** Copyright 2020–2022, Veronica Berglyd Olsen
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

#define CNF_MAIN_WINDOW_SIZE "GuiMain/windowSize"
#define CNF_MAIN_SPLIT_SIZES "GuiMain/mainSplitSizes"

#define CNF_TEXT_FONT_SIZE "TextFormat/fontSize"

#include <QList>
#include <QSize>
#include <QVariant>
#include <QSettings>
#include <QVariantList>
#include <QTextCharFormat>
#include <QCoreApplication>
#include <QTextBlockFormat>

namespace Collett {

/**
 * Converter Functions
 * ===================
 */

QList<int> variantListToInt(const QVariantList &list) {
    QList<int> result;
    for (const QVariant &val : list) {
        result.append(val.toInt());
    }
    return result;
}

QVariantList intListToVariant(const QList<int> &list) {
    QVariantList result;
    for (const int &val : list) {
        result.append(val);
    }
    return result;
}

/**
 * Class Constructor/Destructor/Instance
 * =====================================
 */

CollettSettings *CollettSettings::staticInstance = nullptr;
CollettSettings *CollettSettings::instance() {
    if (staticInstance == nullptr) {
        staticInstance = new CollettSettings();
        qDebug() << "Constructor: CollettSettings";
    }
    return staticInstance;
}

void CollettSettings::destroy() {
    if (staticInstance != nullptr) {
        qDebug() << "Destructor: Static CollettSettings";
        delete CollettSettings::staticInstance;
    }
}

CollettSettings::CollettSettings() {

    // Load Settings
    QSettings settings;

    // GUI Settings
    // ------------

    m_mainWindowSize = settings.value(CNF_MAIN_WINDOW_SIZE, QSize(1200, 800)).toSize();
    m_mainSplitSizes = variantListToInt(settings.value(CNF_MAIN_SPLIT_SIZES, QVariantList() << 300 << 700).toList());

    // Check Values
    if (m_mainWindowSize.width() < 400) {
        m_mainWindowSize.setWidth(400);
    }
    if (m_mainWindowSize.height() < 300) {
        m_mainWindowSize.setHeight(300);
    }

    // Text Format
    // -----------

    m_textFontSize = settings.value(CNF_TEXT_FONT_SIZE, (qreal)13.0).toReal();

    // Check Values
    if (m_textFontSize < 5.0) {
        m_textFontSize = 5.0;
    }
    recalculateTextFormats();

}

CollettSettings::~CollettSettings() {
    qDebug() << "Destructor: CollettSettings";
}

/**
 * Public Class Methods
 * ====================
 */

void CollettSettings::flushSettings() {

    QSettings settings;

    settings.setValue(CNF_MAIN_WINDOW_SIZE, m_mainWindowSize);
    settings.setValue(CNF_MAIN_SPLIT_SIZES, intListToVariant(m_mainSplitSizes));

    settings.setValue(CNF_TEXT_FONT_SIZE, m_textFontSize);

    qDebug() << "CollettSettings values saved";

    return;
}

/**
 * Setter Functions
 * ================
 */

void CollettSettings::setMainWindowSize(const QSize size) {
    m_mainWindowSize = size;
}

void CollettSettings::setMainSplitSizes(const QList<int> &sizes) {
    m_mainSplitSizes = sizes;
}

void CollettSettings::setTextFontSize(const qreal size) {
    m_textFontSize = size;
    recalculateTextFormats();
}

/**
 * Getter Functions
 * ================
 */

QSize CollettSettings::mainWindowSize() const {
    return m_mainWindowSize;
}

QList<int> CollettSettings::mainSplitSizes() const {
    return m_mainSplitSizes;
}

CollettSettings::TextFormat CollettSettings::textFormat() const {
    return m_textFormat;
}

/**
 * Internal Functions
 * ==================
 */

void CollettSettings::recalculateTextFormats() {

    // Text Formats

    QTextCharFormat  defaultCharFmt;
    QTextBlockFormat defaultBlockFmt;

    // Default Values

    qreal defaultLineHeight = 1.15;
    qreal defaultTopMargin = 0.5 * m_textFontSize;
    qreal defaultBottomMargin = 0.5 * m_textFontSize;

    qreal header1FontSize = 2.2*m_textFontSize;
    qreal header2FontSize = 1.9*m_textFontSize;
    qreal header3FontSize = 1.6*m_textFontSize;
    qreal header4FontSize = 1.3*m_textFontSize;

    qreal headerBottomMargin = 0.7 * m_textFontSize;

    // Text Format Values

    m_textFormat.fontSize = m_textFontSize;
    m_textFormat.textIndent = 2.0 * m_textFontSize;
    m_textFormat.lineHeight = 1.15;

    // Default Text Formats

    defaultBlockFmt.setHeadingLevel(0);
    defaultBlockFmt.setLineHeight(defaultLineHeight, QTextBlockFormat::SingleHeight);
    defaultBlockFmt.setTopMargin(defaultTopMargin);
    defaultBlockFmt.setBottomMargin(defaultBottomMargin);
    defaultBlockFmt.setTextIndent(0.0);
    m_textFormat.blockDefault = defaultBlockFmt;

    defaultCharFmt.setFontPointSize(m_textFontSize);
    m_textFormat.charDefault = defaultCharFmt;

    // Paragraph Formats

    m_textFormat.blockParagraph = defaultBlockFmt;
    m_textFormat.charParagraph = defaultCharFmt;

    // Header 1 Formats

    m_textFormat.blockHeader1 = defaultBlockFmt;
    m_textFormat.blockHeader1.setHeadingLevel(1);
    m_textFormat.blockHeader1.setTopMargin(header1FontSize);
    m_textFormat.blockHeader1.setBottomMargin(headerBottomMargin);

    m_textFormat.charHeader1 = defaultCharFmt;
    m_textFormat.charHeader1.setFontPointSize(header1FontSize);

    // Header 2 Formats

    m_textFormat.blockHeader2 = defaultBlockFmt;
    m_textFormat.blockHeader2.setHeadingLevel(2);
    m_textFormat.blockHeader2.setTopMargin(header2FontSize);
    m_textFormat.blockHeader2.setBottomMargin(headerBottomMargin);

    m_textFormat.charHeader2 = defaultCharFmt;
    m_textFormat.charHeader2.setFontPointSize(header2FontSize);

    // Header 3 Formats

    m_textFormat.blockHeader3 = defaultBlockFmt;
    m_textFormat.blockHeader3.setHeadingLevel(3);
    m_textFormat.blockHeader3.setTopMargin(header3FontSize);
    m_textFormat.blockHeader3.setBottomMargin(headerBottomMargin);

    m_textFormat.charHeader3 = defaultCharFmt;
    m_textFormat.charHeader3.setFontPointSize(header3FontSize);

    // Header 4 Formats

    m_textFormat.blockHeader4 = defaultBlockFmt;
    m_textFormat.blockHeader4.setHeadingLevel(4);
    m_textFormat.blockHeader4.setTopMargin(header4FontSize);
    m_textFormat.blockHeader4.setBottomMargin(headerBottomMargin);

    m_textFormat.charHeader4 = defaultCharFmt;
    m_textFormat.charHeader4.setFontPointSize(header4FontSize);

}

} // namespace Collett
