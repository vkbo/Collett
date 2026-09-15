/*
** Collett - Core Data Class
** =========================
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

#include "data.h"
#include "project.h"
#include "settings.h"

#include <QWidget>
#include <QVariant>

namespace Collett {

// Constructor/Destructor/Instance
// ===============================

SharedData *SharedData::staticInstance = nullptr;
SharedData *SharedData::instance()
{
    if (staticInstance == nullptr) {
        staticInstance = new SharedData();
        qDebug() << "Constructor: SharedData";
    }
    return staticInstance;
}

void SharedData::destroy()
{
    if (staticInstance != nullptr) {
        qDebug() << "Destructor: Static SharedData";
        delete SharedData::staticInstance;
        SharedData::staticInstance = nullptr;
    }
}

SharedData::SharedData(QObject *parent) : QObject(parent)
{
    m_spelling = new SpellChecker(this);
}

SharedData::~SharedData()
{
    qDebug() << "Destructor: SharedData";
    m_spelling->setStorage(nullptr);
    m_project.reset();
}

// Public Methods
// ==============

bool SharedData::openProject(const QString &path)
{

    this->closeProject();
    m_project.reset(new Project());
    if (!m_project.data()->hasError()) {
        m_project.data()->openProject(path);
    }
    if (!m_project.data()->isValid()) {
        m_project.reset(nullptr);
        return false;
    }

    m_spelling->setStorage(m_project.data()->store());
    this->updateSpellCheckLanguage();

    emit projectLoaded();

    return true;
}

bool SharedData::saveProject()
{
    if (hasProject()) {
        return m_project.data()->saveProject();
    } else {
        return false;
    }
}

bool SharedData::saveProjectAs(const QString &path)
{
    if (hasProject()) {
        return m_project.data()->saveProjectAs(path);
    } else {
        return false;
    }
}

void SharedData::closeProject()
{
    // The user dictionary points at the project's storage, so it must let go
    // before the project is destroyed
    m_spelling->setStorage(nullptr);
    m_project.reset(nullptr);
}

/**! @brief Load the spell check language from the settings.
 *
 * The project's own language, if set, overrides the global setting. The
 * dictionary is only reloaded when the language actually changes.
 */
void SharedData::updateSpellCheckLanguage()
{
    QString language = Settings::instance()->spellLanguage();
    if (this->hasProject() && m_project.data()->data()->hasSpellLanguage()) {
        language = m_project.data()->data()->spellLanguage();
    }
    if (language != m_spelling->requestedLanguage()) {
        m_spelling->setLanguage(language);
    }
}

// Getters
// =======

bool SharedData::hasProject() const
{
    if (m_project.isNull()) {
        return false;
    } else {
        return m_project.data()->isValid();
    }
}

Project *SharedData::project()
{
    if (hasProject()) {
        return m_project.data();
    } else {
        return nullptr;
    }
}

} // namespace Collett
