/*
Collett – GUI Story Tree Class
==============================

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
#include "storytree.h"
#include "storymodel.h"

#include <QList>
#include <QObject>
#include <QTreeView>

namespace Collett {

GuiStoryTree::GuiStoryTree(QWidget *parent)
    : QTreeView(parent)
{
}

/**
 * Class Methods
 * =============
 */

void GuiStoryTree::restoreTreeState() {
    CollettSettings *mainConf = CollettSettings::instance();

    QList<int> columns = mainConf->storyTreeColSizes();
    for (int i=0; i<COL_STORY_TREE_COL_COUNT; ++i) {
        int width = columns.at(i);
        this->setColumnWidth(i, width > 25 ? width : 25);
    }
}

void GuiStoryTree::saveTreeState() {
    CollettSettings *mainConf = CollettSettings::instance();

    QList<int> columns;
    for (int i=0; i<COL_STORY_TREE_COL_COUNT; ++i) {
        columns.append(this->columnWidth(i));
    }
    mainConf->setStoryTreeColSizes(columns);
}

} // namespace Collett
