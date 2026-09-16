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

#pragma once

#include "collett.h"
#include "mcombobox.h"
#include "mpagedsidebar.h"
#include "mpushbutton.h"
#include "mswitch.h"
#include "scrollableform.h"
#include "settings.h"

#include <QDialog>
#include <QFont>
#include <QLabel>
#include <QLineEdit>
#include <QStackedWidget>
#include <QWidget>

namespace Collett {

// Pages
// =====

class PrefsAppearancePage : public MScrollableForm
{
    Q_OBJECT

public:
    explicit PrefsAppearancePage(QWidget *parent = nullptr);

    // Getters
    QFont guiFont() const { return m_guiFont; };
    QFont textFont() const { return m_textFont; };
    QFont monoFont() const { return m_monoFont; };

    MComboBox *themeMode;
    MComboBox *lightTheme;
    MComboBox *darkTheme;
    MSwitch *nativeFontDialog;
    QLineEdit *guiFontEdit;
    QLineEdit *textFontEdit;
    QLineEdit *monoFontEdit;

private:
    QFont m_guiFont;
    QFont m_textFont;
    QFont m_monoFont;

    // Helpers
    QLineEdit *addFontRow(const QString &label, const QString &helpText, const QFont &font, void (PrefsAppearancePage::*slot)());
    void selectFont(QFont &font, QLineEdit *edit);

private slots:
    void selectGuiFont();
    void selectTextFont();
    void selectMonoFont();
};

// Dialog
// ======

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    // The ids double as the index of the page in the stack, so they must
    // match the order the pages are added in.
    enum Page
    {
        AppearancePage = 0,
    };

    explicit PreferencesDialog(QWidget *parent = nullptr);
    ~PreferencesDialog();

    // Methods
    void done(int result) override;

private:
    Settings *m_settings;

    // Components
    QLabel *m_titleLabel;
    MPagedSideBar *m_sideBar;
    QStackedWidget *m_stack;
    MPushButton *m_btnSave;
    MPushButton *m_btnCancel;

    // Pages
    PrefsAppearancePage *m_appearancePage;

private slots:
    void onSideBarClicked(int pageId);
    void doSave();
};
} // namespace Collett
