/*
** Collett – Edit Item Dialog
** ==========================
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
#include "edititem.h"
#include "node.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

namespace Collett {

// Constructor/Destructor
// ======================

EditItemDialog::EditItemDialog(QWidget *parent, Node *node) : QDialog(parent) {

    this->setWindowTitle(tr("Edit Item"));

    m_titleValue = new QLineEdit(this);
    m_titleValue->setMaxLength(200);
    m_titleValue->setText(node->name());
    m_titleValue->selectAll();

    QLabel *titleLabel = new QLabel(tr("Name"), this);

    QPushButton *okButton = new QPushButton(tr("Ok"), this);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), this);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    buttonBox->addButton(cancelButton, QDialogButtonBox::RejectRole);

    QHBoxLayout *innerBox = new QHBoxLayout();
    innerBox->addWidget(titleLabel, 0);
    innerBox->addWidget(m_titleValue, 1);
    innerBox->setSpacing(12);

    QVBoxLayout *outerBox = new QVBoxLayout();
    outerBox->addLayout(innerBox);
    outerBox->addWidget(buttonBox);
    outerBox->setSpacing(12);

    this->setLayout(outerBox);
    this->setMinimumWidth(300);

    this->connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    this->connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

EditItemDialog::~EditItemDialog() {
    qDebug() << "Destructor: EditItemDialog";
}

void EditItemDialog::editNode(QWidget *parent, Node *node) {
    QPointer<EditItemDialog> dialog(new EditItemDialog(parent, node));
    dialog->exec();
    if (dialog->result() == QDialog::Accepted) {
        node->setName(dialog->m_titleValue->text());
    }
    dialog->deleteLater();
}

} // namespace Collett
