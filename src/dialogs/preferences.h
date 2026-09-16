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
#include "mpagedsidebar.h"
#include "mpushbutton.h"
#include "scrollableform.h"
#include "settings.h"

#include <QComboBox>
#include <QDialog>
#include <QLabel>
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

    QComboBox *themeMode;
    QComboBox *lightTheme;
    QComboBox *darkTheme;
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
