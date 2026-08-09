/*
** Collett - Main Function
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

#include <iomanip>
#include <iostream>

#include "collett.h"
#include "guimain.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>

// ANSI colours for the log output
namespace {
constexpr const char *ANSI_BLUE = "\033[94m";
constexpr const char *ANSI_GREEN = "\033[92m";
constexpr const char *ANSI_YELLOW = "\033[93m";
constexpr const char *ANSI_RED = "\033[91m";
constexpr const char *ANSI_WHITE = "\033[97m";
constexpr const char *ANSI_RESET = "\033[0m";
constexpr int LOG_FILE_WIDTH = 18;
constexpr int LOG_LINE_WIDTH = 4;
} // namespace

/**!
 * @brief Log message handler&
 *
 * Custom message handler that prints a timestamp, and if DEBUG is enabled the
 * source file name and line number aligned on the colon, followed by the log
 * level and the message. The file name, line number and log level are coloured
 * with ANSI escapes unless the NOCOLOR environment variable is set.
 *
 * @param type    the message type.
 * @param context the message context.
 * @param msg     the message text.
 */
void collettLogHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{

#ifndef DEBUG
    if (type == QtDebugMsg)
        return;
#endif

    static const bool useColor = !qEnvironmentVariableIsSet("NOCOLOR");

    QString time = QDateTime::currentDateTime().toString(Qt::ISODateWithMs);

    const char *color = "";
    const char *label = "";
    switch (type) {
    case QtDebugMsg:
        color = ANSI_BLUE;
        label = "DEBUG     ";
        break;
    case QtInfoMsg:
        color = ANSI_GREEN;
        label = "INFO      ";
        break;
    case QtWarningMsg:
        color = ANSI_YELLOW;
        label = "WARNING   ";
        break;
    case QtCriticalMsg:
        color = ANSI_RED;
        label = "CRITICAL  ";
        break;
    case QtFatalMsg:
        color = ANSI_RED;
        label = "FATAL     ";
        break;
    }

    std::cout << "[" << time.toStdString() << "]  ";
#ifdef DEBUG
    QFileInfo file(context.file ? context.file : "");
    std::string fileName = file.fileName().toStdString();
    if (useColor) {
        std::cout << ANSI_BLUE << std::setw(LOG_FILE_WIDTH) << std::right << fileName << ANSI_RESET << ":"
                  << ANSI_WHITE << std::setw(LOG_LINE_WIDTH) << std::left << context.line << ANSI_RESET << "  ";
    } else {
        std::cout << std::setw(LOG_FILE_WIDTH) << std::right << fileName << ":"
                  << std::setw(LOG_LINE_WIDTH) << std::left << context.line << "  ";
    }
#endif
    if (useColor) {
        std::cout << color << label << ANSI_RESET;
    } else {
        std::cout << label;
    }
    std::cout << msg.toStdString() << std::endl;
}

int main(int argc, char *argv[])
{

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
        QCoreApplication::translate("main", "path"));
    parser.addOption(openPath);
    parser.process(app);

    Collett::GuiMain mainGUI;
    mainGUI.show();
    if (parser.isSet(openPath)) {
        mainGUI.openProject(parser.value(openPath));
    }

    // Styles
    QFile styles(":/styles/styles.qss");
    if (styles.open(QFile::ReadOnly)) {
        app.setStyleSheet(QLatin1String(styles.readAll()));
    }

    return app.exec();
}
