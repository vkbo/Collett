/*
** Collett – Main Function
** =======================
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

#include <iostream>

#include "collett.h"
#include "guimain.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>

/**!
 * @brief Log message handler
 *
 * Custom message handler that adds a timestamp and log level to the log message
 * as well as a filename and line number if DEBUG is enabled.
 *
 * @param type    the message type.
 * @param context the message context.
 * @param msg     the message text.
 */
void collettLogHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {

#ifndef DEBUG
    if (type == QtDebugMsg) return;
#endif

    QString time = QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
    QFileInfo file(context.file ? context.file : "");

    std::cout << "[" << time.toStdString() << "] ";
    switch (type) {
        case QtDebugMsg:    std::cout << "DEBUG     "; break;
        case QtInfoMsg:     std::cout << "INFO      "; break;
        case QtWarningMsg:  std::cout << "WARNING   "; break;
        case QtCriticalMsg: std::cout << "CRITICAL  "; break;
        case QtFatalMsg:    std::cout << "FATAL     "; break;
    }
    std::cout << msg.toStdString();
#ifdef DEBUG
    std::cout << " [" << file.fileName().toStdString() << ":" << context.line << "]";
#endif
    std::cout << std::endl;
}

int main(int argc, char *argv[]) {

    qInstallMessageHandler(collettLogHandler);
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Collett");
    QCoreApplication::setOrganizationDomain("vkbo.net");
    QCoreApplication::setApplicationName("Collett");
    QCoreApplication::setApplicationVersion(COL_VERSION_STR);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption openPath(
        QStringList() << "o" << "open",
        QCoreApplication::translate("main", "Open the <path> project on launch."),
        QCoreApplication::translate("main", "path")
    );
    parser.addOption(openPath);
    parser.process(app);

    Collett::GuiMain mainGUI;
    mainGUI.show();
    if (parser.isSet(openPath)) {
        mainGUI.openProject(parser.value(openPath));
    }

    // Styles
    QFile styles(":/styles/styles.qss");
    styles.open(QFile::ReadOnly);
    app.setStyleSheet(QLatin1String(styles.readAll()));

    return app.exec();
}
