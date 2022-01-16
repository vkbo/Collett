/*
** Collett – Project XML Reader Class
** ==================================
**
** This file is a part of Collett
** Copyright 2020–2021, Veronica Berglyd Olsen
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
#include "projectxmlreader.h"

#include <QString>
#include <QXmlStreamReader>

namespace Collett {

ProjectXmlReader::ProjectXmlReader(Project *project)
    : m_project(project)
{
    m_lastError = "";
}

ProjectXmlReader::~ProjectXmlReader() {
    qDebug() << "Destructor: ProjectXmlReader";
}

bool ProjectXmlReader::readProjectFile() {

    QString filePath = m_project->projectPath();
    QFileInfo fInfo(filePath);

    if (!fInfo.isFile()) {
        m_lastError = tr("Could not read project file. Path is not a file: %1").arg(filePath);
        qWarning() << "Could not read project file. Path is not a file:" << filePath;
        return false;
    }

    if (!fInfo.isReadable()) {
        m_lastError = tr("Could not read project file. Path is not readable: %1").arg(filePath);
        qWarning() << "Could not read project file. Path is not readable:" << filePath;
        return false;
    }

    QFile inFile(filePath);
    if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = tr("Could not read project file: %1").arg(filePath);
        qWarning() << "Could not read project file:" << filePath;
        return false;
    }

    QXmlStreamReader xml(&inFile);
    int docStatus = 0b0000;
    while (!xml.atEnd()) {

        switch (xml.readNext()) {

        case QXmlStreamReader::StartDocument:
            docStatus |= 0b0001;
            break;

        case QXmlStreamReader::EndDocument:
            docStatus |= 0b0010;
            break;

        case QXmlStreamReader::StartElement:
            qDebug() << xml.lineNumber() << "Opening" << xml.qualifiedName();
            if (xml.qualifiedName() == QLatin1String("collett:project")) {
                docStatus |= 0b0100;
            }
            else if (xml.qualifiedName() == QLatin1String("collett:meta")) {

            } 
            break;

        case QXmlStreamReader::EndElement:
            qDebug() << xml.lineNumber() << "Closing" << xml.qualifiedName();
            if (xml.qualifiedName() == QLatin1String("collett:project")) {
                docStatus |= 0b1000;
            } else {
                qWarning() << "XML element" << xml.qualifiedName() << "was not fully parsed";
            }
            break;

        case QXmlStreamReader::Characters:
            if (!xml.isWhitespace()) {
                qWarning() << "Unparsed text content on line" << xml.lineNumber() << ">" << xml.text();
            }
            break;

        default:
            qWarning() << "Unexpected XML content on line" << xml.lineNumber();
            break;
        }
    }
    if (xml.hasError()) {
    }

    qDebug() << "Document status:" << docStatus;
    inFile.close();

    return docStatus == 0b1111;
}

/**
 * Section Readers
 * ===============
 */

void ProjectXmlReader::readMetaXML(QXmlStreamReader &xml) {

    Q_ASSERT(xml.qualifiedName() == QLatin1String("project:meta"));

    return;
}


} // namespace Collett
