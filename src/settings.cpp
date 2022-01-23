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
 * Private Class Declaration
 * =========================
 */

class CollettSettingsPrivate
{
public:
    static CollettSettings *instance;

    CollettSettingsPrivate() {};
    ~CollettSettingsPrivate() {};

    QSize      m_mainWindowSize;
    QList<int> m_mainSplitSizes;
};

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
 * Public Class Contructor/Destructor
 * ==================================
 */

CollettSettings *CollettSettingsPrivate::instance = nullptr;

CollettSettings *CollettSettings::instance() {
    if (CollettSettingsPrivate::instance == nullptr) {
        CollettSettingsPrivate::instance = new CollettSettings();
        qDebug() << "Constructor: CollettSettings";
    }
    return CollettSettingsPrivate::instance;
}

void CollettSettings::destroy() {
    if (CollettSettingsPrivate::instance != nullptr) {
        delete CollettSettingsPrivate::instance;
    }
}

CollettSettings::CollettSettings()
    : d_ptr(new CollettSettingsPrivate())
{
    Q_D(CollettSettings);

    // Load Settings
    QSettings settings;

    d->m_mainWindowSize = settings.value(CNF_MAIN_WINDOW_SIZE, QSize(1200, 800)).toSize();
    d->m_mainSplitSizes = variantListToInt(settings.value(CNF_MAIN_SPLIT_SIZES, QVariantList() << 300 << 700).toList());

    // Check Values
    if (d->m_mainWindowSize.width() < 400) {
        d->m_mainWindowSize.setWidth(400);
    }
    if (d->m_mainWindowSize.height() < 300) {
        d->m_mainWindowSize.setHeight(300);
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
    Q_D(CollettSettings);

    QSettings settings;

    settings.setValue(CNF_MAIN_WINDOW_SIZE, d->m_mainWindowSize);
    settings.setValue(CNF_MAIN_SPLIT_SIZES, intListToVariant(d->m_mainSplitSizes));

    qDebug() << "CollettSettings values saved";

    return;
}

/**
 * Setter Functions
 * ================
 */

void CollettSettings::setMainWindowSize(const QSize size) {
    Q_D(CollettSettings);
    d->m_mainWindowSize = size;
}

void CollettSettings::setMainSplitSizes(const QList<int> &sizes) {
    Q_D(CollettSettings);
    d->m_mainSplitSizes = sizes;
}

/**
 * Getter Functions
 * ================
 */

QSize CollettSettings::mainWindowSize() const {
    Q_D(const CollettSettings);
    return d->m_mainWindowSize;
}

QList<int> CollettSettings::mainSplitSizes() const {
    Q_D(const CollettSettings);
    return d->m_mainSplitSizes;
}

} // namespace Collett
