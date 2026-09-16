/*
** Collett - Modified QComboBox Class
** ==================================
**
** This file is a part of Collett
** Copyright (C) 2026 Veronica Berglyd Olsen
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
#include "mcombobox.h"

#include <QComboBox>
#include <QVariant>
#include <QWheelEvent>
#include <QWidget>

namespace Collett {

// Constructor/Destructor
// ======================

/**! @brief A combo box that leaves the mouse wheel to the page it sits on.
 *
 * On a scrollable form, wheel events passing over a combo box would change
 * its value instead of scrolling. This one only reacts to the wheel while it
 * has keyboard focus.
 */
MComboBox::MComboBox(QWidget *parent, int maxItems) : QComboBox(parent)
{
    this->setFocusPolicy(Qt::StrongFocus);
    this->setMaxVisibleItems(maxItems);
}

MComboBox::~MComboBox() {}

// Setters
// =======

/**! @brief Select the item with the given data, or the fallback if it is missing.
 */
void MComboBox::setCurrentData(const QVariant &data, const QVariant &fallback)
{
    const int index = this->findData(data);
    this->setCurrentIndex(index < 0 ? this->findData(fallback) : index);
}

// Events
// ======

/**! @brief Only handle the mouse wheel when the widget has focus.
 */
void MComboBox::wheelEvent(QWheelEvent *event)
{
    if (this->hasFocus()) {
        QComboBox::wheelEvent(event);
    } else {
        event->ignore();
    }
}

} // namespace Collett
