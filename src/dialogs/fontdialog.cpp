/*
** Collett - Font Dialog
** =====================
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
#include "fontdialog.h"
#include "mpushbutton.h"
#include "settings.h"
#include "theme.h"

#include <QDialogButtonBox>
#include <QFont>
#include <QFontDialog>
#include <QPointer>
#include <QString>
#include <QWidget>

namespace Collett {

// Constructor/Destructor
// ======================

/**! @brief The Qt font dialog with the standard buttons and a remembered size.
 *
 * The native dialog is never used here, since its buttons and size cannot be
 * controlled. Use selectFont to pick between this and the native one.
 */
FontDialog::FontDialog(const QFont &initial, QWidget *parent) : QFontDialog(initial, parent)
{
    m_settings = Settings::instance();
    Theme *theme = Theme::instance();

    this->setOption(QFontDialog::DontUseNativeDialog, true);
    this->resize(m_settings->fontWindowSize());

    // Replace the buttons, keeping the box so its accept and reject signals
    // still reach the dialog
    if (QDialogButtonBox *buttonBox = this->findChild<QDialogButtonBox *>()) {
        MPushButton *btnOk = theme->getStandardButton(StandardButton::OkButton, this);
        MPushButton *btnCancel = theme->getStandardButton(StandardButton::CancelButton, this);
        buttonBox->clear();
        buttonBox->addButton(btnOk, QDialogButtonBox::AcceptRole);
        buttonBox->addButton(btnCancel, QDialogButtonBox::RejectRole);
        btnOk->setDefault(true);
    }
}

FontDialog::~FontDialog()
{
    qDebug() << "Destructor: FontDialog";
}

// Static Methods
// ==============

/**! @brief Let the user pick a font, using the native dialog if asked.
 *
 * Returns the chosen font and sets ok to whether the dialog was accepted. On
 * a rejected dialog the initial font is returned.
 */
QFont FontDialog::selectFont(const QFont &initial, QWidget *parent, const QString &title, bool native, bool *ok)
{
    if (native) {
#ifdef Q_OS_MACOS
        // The macOS font panel has no accept button, so closing it must count
        // as accepting the current font
        QFontDialog dialog(initial, parent);
        dialog.setWindowTitle(title);
        dialog.exec();
        *ok = true;
        return dialog.currentFont();
#else
        return QFontDialog::getFont(ok, initial, parent, title);
#endif
    }

    QPointer<FontDialog> dialog(new FontDialog(initial, parent));
    dialog->setWindowTitle(title);
    dialog->exec();

    *ok = dialog->result() == QDialog::Accepted;
    const QFont font = *ok ? dialog->selectedFont() : initial;
    dialog->deleteLater();
    return font;
}

// Public Methods
// ==============

/**! @brief Remember the window size when the dialog closes.
 */
void FontDialog::done(int result)
{
    m_settings->setFontWindowSize(this->size());
    QFontDialog::done(result);
}

} // namespace Collett
