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
#include "mpagedsidebar.h"
#include "mpushbutton.h"
#include "preferences.h"
#include "settings.h"
#include "theme.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPalette>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

namespace Collett {

namespace {
// Font scale of the dialog title relative to the default font
constexpr qreal titleScale = 1.25;
} // namespace

// Constructor/Destructor
// ======================

PreferencesDialog::PreferencesDialog(QWidget *parent) : QDialog(parent)
{
    Theme *theme = Theme::instance();
    m_settings = Settings::instance();

    const QColor helpColor = theme->helpTextColor();

    this->setWindowTitle(tr("Preferences"));
    this->setMinimumSize(600, 500);
    this->resize(m_settings->prefsWindowSize());

    // Title
    m_titleLabel = new QLabel(tr("Preferences"), this);
    m_titleLabel->setIndent(4);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSizeF(titleScale * titleFont.pointSizeF());
    m_titleLabel->setFont(titleFont);
    QPalette titlePalette = m_titleLabel->palette();
    titlePalette.setColor(QPalette::WindowText, helpColor);
    m_titleLabel->setPalette(titlePalette);

    // Side Bar
    m_sideBar = new MPagedSideBar(this);
    m_sideBar->setLabelColor(helpColor);
    m_sideBar->setAccessibleName(m_titleLabel->text());
    m_sideBar->addLabel(tr("Application"));
    m_sideBar->addButton(tr("General"), Page::GeneralPage);
    m_sideBar->addButton(tr("Appearance"), Page::AppearancePage);
    m_sideBar->addLabel(tr("Editor"));
    m_sideBar->addButton(tr("Text Editor"), Page::TextEditorPage);
    m_sideBar->addButton(tr("Spell Checking"), Page::SpellCheckPage);

    // Pages
    // Added in the order of the Page enum, which is the stack index
    m_stack = new QStackedWidget(this);
    m_stack->addWidget(new QWidget(this)); // Page::GeneralPage
    m_stack->addWidget(new QWidget(this)); // Page::AppearancePage
    m_stack->addWidget(new QWidget(this)); // Page::TextEditorPage
    m_stack->addWidget(new QWidget(this)); // Page::SpellCheckPage

    // Buttons
    m_btnSave = theme->getStandardButton(StandardButton::SaveButton, this);
    m_btnCancel = theme->getStandardButton(StandardButton::CancelButton, this);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(m_btnSave, QDialogButtonBox::AcceptRole);
    buttonBox->addButton(m_btnCancel, QDialogButtonBox::RejectRole);

    // Assemble
    QHBoxLayout *topBox = new QHBoxLayout();
    topBox->addWidget(m_titleLabel);
    topBox->addStretch(1);

    QHBoxLayout *mainBox = new QHBoxLayout();
    mainBox->addWidget(m_sideBar);
    mainBox->addWidget(m_stack, 1);
    mainBox->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *outerBox = new QVBoxLayout();
    outerBox->addLayout(topBox);
    outerBox->addLayout(mainBox, 1);
    outerBox->addWidget(buttonBox);
    outerBox->setSpacing(8);

    this->setLayout(outerBox);
    this->setSizeGripEnabled(true);

    connect(m_sideBar, &MPagedSideBar::buttonClicked, this, &PreferencesDialog::onSideBarClicked);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &PreferencesDialog::doSave);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Open on the first page
    m_sideBar->setSelected(Page::GeneralPage);
    this->onSideBarClicked(Page::GeneralPage);
}

PreferencesDialog::~PreferencesDialog()
{
    qDebug() << "Destructor: PreferencesDialog";
}

// Public Methods
// ==============

/**! @brief Remember the window size when the dialog closes.
 *
 * All ways of closing the dialog end up here, including the window close
 * button, which rejects the dialog.
 */
void PreferencesDialog::done(int result)
{
    m_settings->setPrefsWindowSize(this->size());
    QDialog::done(result);
}

// Private Slots
// =============

/**! @brief Show the page matching the clicked side bar button.
 */
void PreferencesDialog::onSideBarClicked(int pageId)
{
    if (pageId >= 0 && pageId < m_stack->count()) {
        m_stack->setCurrentIndex(pageId);
    }
}

/**! @brief Save the values set in the form and close the dialog.
 */
void PreferencesDialog::doSave()
{
    this->accept();
}

} // namespace Collett
