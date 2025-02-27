/*
** Collett – GUI Project Tree Class
** ================================
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

#include "collett.h"
#include "mtreeview.h"
#include "projectview.h"
#include "projectmodel.h"

#include <QTreeView>
#include <QWidget>
#include <QItemSelectionModel>

namespace Collett {

// Constructor/Destructor
// ======================

GuiProjectView::GuiProjectView(QWidget *parent) : MTreeView(parent) {

    m_data = SharedData::instance();
}

GuiProjectView::~GuiProjectView() {
    qDebug() << "Destructor: GuiProjectView";
}

// Public Methods
// ==============

void GuiProjectView::openProjectTasks() {
    if (m_data->hasProject()) {
        ProjectModel *model = m_data->project()->tree()->model();
        if (model) {
            QItemSelectionModel *m = this->selectionModel();
            this->setModel(model);
            delete m;
        }
    }
}

void GuiProjectView::closeProjectTasks() {
    QItemSelectionModel *m = this->selectionModel();
    this->setModel(nullptr);
    delete m;
}

} // namespace Collett
