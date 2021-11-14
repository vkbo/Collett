/*
Collett – Main Settings Class
=============================

This file is a part of Collett
Copyright 2020–2021, Veronica Berglyd Olsen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "collett.h"
#include "settings.h"

#define CNF_MAIN_WINDOW_SIZE        "GuiMain/windowSize"
#define CNF_MAIN_SPLIT_TREE_WIDTH   "GuiMain/splitTreeWidth"
#define CNF_MAIN_SPLIT_EDITOR_WIDTH "GuiMain/splitEditorWidth"

#include <QCoreApplication>
#include <QList>
#include <QSettings>
#include <QSize>
#include <QVariant>

#include <QDebug>

namespace Collett {

/*
    Private Class Declaration
    =========================
*/

class CollettSettingsPrivate
{
public:
    static CollettSettings *instance;

    CollettSettingsPrivate() {};
    ~CollettSettingsPrivate() {};

    QSize m_mainWindowSize;
    int   m_mainSplitTreeWidth;
    int   m_mainSplitEditorWidth;
};

/*
    Public Class Contruction/Deconstruction
    =======================================
*/

CollettSettings *CollettSettingsPrivate::instance = nullptr;

CollettSettings *CollettSettings::instance() {
    if (nullptr == CollettSettingsPrivate::instance) {
        CollettSettingsPrivate::instance = new CollettSettings();
        qDebug() << "CollettSettings instance created";
    }
    return CollettSettingsPrivate::instance;
}

CollettSettings::CollettSettings() : d_ptr(new CollettSettingsPrivate()) {

    Q_D(CollettSettings);

    // Load Settings
    QSettings mainConf;

    d->m_mainWindowSize       = mainConf.value(CNF_MAIN_WINDOW_SIZE,        QSize(1200, 800)).toSize();
    d->m_mainSplitTreeWidth   = mainConf.value(CNF_MAIN_SPLIT_TREE_WIDTH,   QVariant(300)).toInt();
    d->m_mainSplitEditorWidth = mainConf.value(CNF_MAIN_SPLIT_EDITOR_WIDTH, QVariant(900)).toInt();

    // Check Values
    if (d->m_mainWindowSize.width() < 400) {
        d->m_mainWindowSize.setWidth(400);
    }
    if (d->m_mainWindowSize.height() < 300) {
        d->m_mainWindowSize.setHeight(300);
    }
}

CollettSettings::~CollettSettings() {
    flushSettings();
}

/*
    Public Class Methods
    ====================
*/

void CollettSettings::flushSettings() {
    Q_D(CollettSettings);

    QSettings mainConf;

    mainConf.setValue(CNF_MAIN_WINDOW_SIZE,        QVariant(d->m_mainWindowSize));
    mainConf.setValue(CNF_MAIN_SPLIT_TREE_WIDTH,   QVariant(d->m_mainSplitTreeWidth));
    mainConf.setValue(CNF_MAIN_SPLIT_EDITOR_WIDTH, QVariant(d->m_mainSplitEditorWidth));

    qDebug() << "CollettSettings values saved";

    return;
}

/*
    Setter Functions
    ================
*/

void CollettSettings::setMainWindowSize(const QSize size) {
    Q_D(CollettSettings);
    d->m_mainWindowSize = size;
    return;
}

void CollettSettings::setMainSplitSizes(const QList<int> &sizes) {
    Q_D(CollettSettings);
    if (sizes.length() > 0) {
        d->m_mainSplitTreeWidth = sizes.at(0);
    }
    if (sizes.length() > 1) {
        d->m_mainSplitEditorWidth = sizes.at(1);
    }
}

/*
    Getter Functions
    ================
*/

QSize CollettSettings::mainWindowSize() const {
    Q_D(const CollettSettings);
    return d->m_mainWindowSize;
}

QList<int> CollettSettings::mainSplitSizes() const {
    Q_D(const CollettSettings);
    QList<int> sizes;
    sizes.append(d->m_mainSplitTreeWidth);
    sizes.append(d->m_mainSplitEditorWidth);
    return sizes;
}

} // namespace Collett
