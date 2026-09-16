/*
** Collett - Preferences Dialog
** ============================
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
#include "mpushbutton.h"
#include "preferences.h"
#include "theme.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QWidget>

namespace Collett {

// Constructor/Destructor
// ======================

PreferencesDialog::PreferencesDialog(QWidget *parent) : QDialog(parent)
{
    Theme *theme = Theme::instance();

    this->setWindowTitle(tr("Preferences"));
    this->setMinimumSize(600, 500);

    // Buttons
    m_btnSave = theme->getStandardButton(StandardButton::SaveButton, this);
    m_btnCancel = theme->getStandardButton(StandardButton::CancelButton, this);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(m_btnSave, QDialogButtonBox::AcceptRole);
    buttonBox->addButton(m_btnCancel, QDialogButtonBox::RejectRole);

    // Assemble
    QVBoxLayout *outerBox = new QVBoxLayout();
    outerBox->addStretch(1);
    outerBox->addWidget(buttonBox);
    outerBox->setSpacing(12);

    this->setLayout(outerBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &PreferencesDialog::doSave);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

PreferencesDialog::~PreferencesDialog()
{
    qDebug() << "Destructor: PreferencesDialog";
}

// Private Slots
// =============

/**! @brief Save the values set in the form and close the dialog.
 */
void PreferencesDialog::doSave()
{
    this->accept();
}

} // namespace Collett
