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
#include "miconbutton.h"
#include "mpagedsidebar.h"
#include "mpushbutton.h"
#include "mswitch.h"
#include "preferences.h"
#include "scrollableform.h"
#include "settings.h"
#include "theme.h"
#include "tools.h"

#include <QComboBox>
#include <QFont>
#include <QFontDialog>
#include <QLineEdit>
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

// Pages
// =====

PrefsAppearancePage::PrefsAppearancePage(QWidget *parent) : MScrollableForm(parent)
{
    Settings *settings = Settings::instance();
    Theme *theme = Theme::instance();

    this->addGroupLabel(tr("Colour Theme"));

    themeMode = new QComboBox(this);
    themeMode->setMinimumWidth(200);
    themeMode->addItem(tr("Follow Desktop"), ThemeMode::AutoTheme);
    themeMode->addItem(tr("Light"), ThemeMode::LightTheme);
    themeMode->addItem(tr("Dark"), ThemeMode::DarkTheme);
    themeMode->setCurrentIndex(qMax(0, themeMode->findData(settings->themeMode())));
    this->addRow(tr("Theme mode"), themeMode, tr("Use the light or dark colour theme, or switch with the desktop."));

    lightTheme = new QComboBox(this);
    lightTheme->setMinimumWidth(200);
    for (const ThemeEntry &entry : theme->lightThemes()) {
        lightTheme->addItem(entry.name, entry.key);
    }
    lightTheme->setCurrentIndex(qMax(0, lightTheme->findData(settings->lightTheme())));
    this->addRow(tr("Light colour theme"), lightTheme, tr("The colour theme used in light mode."));

    darkTheme = new QComboBox(this);
    darkTheme->setMinimumWidth(200);
    for (const ThemeEntry &entry : theme->darkThemes()) {
        darkTheme->addItem(entry.name, entry.key);
    }
    darkTheme->setCurrentIndex(qMax(0, darkTheme->findData(settings->darkTheme())));
    this->addRow(tr("Dark colour theme"), darkTheme, tr("The colour theme used in dark mode."));

    this->addGroupLabel(tr("Fonts"));

    nativeFontDialog = new MSwitch(this);
    nativeFontDialog->setChecked(settings->nativeFontDialog());
    this->addRow(tr("Use the system's font selection dialog"), nativeFontDialog, tr("Turn off to use the Qt font dialog, which may have more options."));

    m_guiFont = settings->guiFont();
    m_textFont = settings->textFont();
    m_monoFont = settings->monoFont();

    guiFontEdit = this->addFontRow(tr("User interface font"), tr("Requires restart to take effect."), m_guiFont, &PrefsAppearancePage::selectGuiFont);
    textFontEdit = this->addFontRow(tr("Text font"), tr("The font used for document text in the editor."), m_textFont, &PrefsAppearancePage::selectTextFont);
    monoFontEdit = this->addFontRow(tr("Monospace font"), tr("The fixed width font used in the editor."), m_monoFont, &PrefsAppearancePage::selectMonoFont);

    this->finalise();
}

/**! @brief Add a row with a read-only font description and a select button.
 */
QLineEdit *PrefsAppearancePage::addFontRow(const QString &label, const QString &helpText, const QFont &font, void (PrefsAppearancePage::*slot)())
{
    QLineEdit *edit = new QLineEdit(this);
    edit->setReadOnly(true);
    edit->setMinimumWidth(200);
    edit->setText(FontUtils::describeFont(font));
    edit->setCursorPosition(0);

    MIconButton *button = Theme::instance()->getIconButton(ToolButton::FontButton, this);
    connect(button, &MIconButton::clicked, this, slot);

    this->addRow(label, edit, helpText, button);
    return edit;
}

/**! @brief Open the font dialog and store the result if accepted.
 *
 * The native dialog switch on the page decides whether the platform dialog
 * or the Qt one is used.
 */
void PrefsAppearancePage::selectFont(QFont &font, QLineEdit *edit)
{
    QFontDialog::FontDialogOptions options;
    if (!nativeFontDialog->isChecked()) {
        options |= QFontDialog::DontUseNativeDialog;
    }
    bool ok = false;
    const QFont selected = QFontDialog::getFont(&ok, font, this, tr("Select Font"), options);
    if (ok) {
        font = selected;
        edit->setText(FontUtils::describeFont(font));
        edit->setCursorPosition(0);
    }
}

void PrefsAppearancePage::selectGuiFont()
{
    this->selectFont(m_guiFont, guiFontEdit);
}

void PrefsAppearancePage::selectTextFont()
{
    this->selectFont(m_textFont, textFontEdit);
}

void PrefsAppearancePage::selectMonoFont()
{
    this->selectFont(m_monoFont, monoFontEdit);
}

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
    m_sideBar->addButton(tr("Appearance"), Page::AppearancePage);

    // Pages
    // Added in the order of the Page enum, which is the stack index
    m_appearancePage = new PrefsAppearancePage(this);

    m_stack = new QStackedWidget(this);
    m_stack->addWidget(m_appearancePage); // Page::AppearancePage

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
    m_sideBar->setSelected(Page::AppearancePage);
    this->onSideBarClicked(Page::AppearancePage);
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
 *
 * The theme is reloaded if the selection changed.
 */
void PreferencesDialog::doSave()
{
    const QVariant themeMode = m_appearancePage->themeMode->currentData();
    const QVariant lightTheme = m_appearancePage->lightTheme->currentData();
    const QVariant darkTheme = m_appearancePage->darkTheme->currentData();

    if (themeMode.isValid()) m_settings->setThemeMode(ThemeMode(themeMode.toInt()));
    if (lightTheme.isValid()) m_settings->setLightTheme(lightTheme.toString());
    if (darkTheme.isValid()) m_settings->setDarkTheme(darkTheme.toString());
    Theme::instance()->loadTheme();

    m_settings->setNativeFontDialog(m_appearancePage->nativeFontDialog->isChecked());
    m_settings->setGuiFont(m_appearancePage->guiFont());
    m_settings->setTextFont(m_appearancePage->textFont());
    m_settings->setMonoFont(m_appearancePage->monoFont());

    this->accept();
}

} // namespace Collett
