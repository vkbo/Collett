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

#include <QList>
#include <QSize>
#include <QVariant>
#include <QSettings>
#include <QVariantList>
#include <QCoreApplication>

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

    m_mainWindowSize = settings.value(CNF_MAIN_WINDOW_SIZE, QSize(1200, 800)).toSize();
    m_mainSplitSizes = variantListToInt(settings.value(CNF_MAIN_SPLIT_SIZES, QVariantList() << 300 << 700).toList());

    // Check Values
    if (m_mainWindowSize.width() < 400) {
        m_mainWindowSize.setWidth(400);
    }
    if (m_mainWindowSize.height() < 300) {
        m_mainWindowSize.setHeight(300);
    }
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

} // namespace Collett
