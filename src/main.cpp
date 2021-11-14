/*
Collett – Main Function
=======================

This file is a part of Collett
Copyright 2021, Veronica Berglyd Olsen

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
#include "guimain.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

int main(int argc, char *argv[]) {

    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Collett");
    QCoreApplication::setOrganizationDomain("vkbo.net");
    QCoreApplication::setApplicationName("Collett");
    QCoreApplication::setApplicationVersion(COL_VERSION_STR);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption openFlag(
        QStringList() << "o" << "open",
        QCoreApplication::translate("main", "Open the <path> project on launch."),
        QCoreApplication::translate("main", "path")
    );
    parser.addOption(openFlag);
    parser.process(app);

    Collett::GuiMain mainGUI;
    mainGUI.show();
    // if (parser.isSet(openFlag)) {
    //     mainGUI.openProject(parser.value(openFlag));
    // }

    return app.exec();
}
