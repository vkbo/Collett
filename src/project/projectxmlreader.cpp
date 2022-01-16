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

#ifdef DEBUG
#define XML_DEBUG(m, x)(qDebug("%s <%s> at %d:%d", m, qPrintable(x.qualifiedName().toString()), int(x.lineNumber()), int(x.columnNumber())))
#else
#define XML_DEBUG(m, x)
#endif
#define XML_WARN_GEN(m, x)(qWarning("%s at %d:%d", m, int(x.lineNumber()), int(x.columnNumber())))
#define XML_WARN_ELEM(m, x)(qWarning("%s <%s> at %d:%d", m, qPrintable(x.qualifiedName().toString()), int(x.lineNumber()), int(x.columnNumber())))

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
    xml.setNamespaceProcessing(true);

    int docStatus = 0b0000;
    while (!xml.atEnd()) {
        switch (xml.readNext()) {

        case QXmlStreamReader::Invalid:
            XML_WARN_GEN("Invalid content", xml);
            break;

        case QXmlStreamReader::StartDocument:
            docStatus |= 0b0001;
            break;

        case QXmlStreamReader::EndDocument:
            docStatus |= 0b0010;
            break;

        case QXmlStreamReader::StartElement:
            XML_DEBUG("Element", xml);
            if (xml.qualifiedName() == QStringLiteral("collett:project")) {
                docStatus |= 0b0100;
            } else if (xml.qualifiedName() == QStringLiteral("collett:meta")) {
                if (!readMetaXML(xml))
                    xml.raiseError("Could not parse <collett:meta> section");
            } else if (xml.qualifiedName() == QStringLiteral("collett:settings")) {
                if (!readSettingsXML(xml))
                    xml.raiseError("Could not parse <collett:settings> section");
            } else if (xml.qualifiedName() == QStringLiteral("collett:styles")) {
                if (!readStylesXML(xml))
                    xml.raiseError("Could not parse <collett:styles> section");
            } else if (xml.qualifiedName() == QStringLiteral("collett:structure")) {
                if (!readStructureXML(xml))
                    xml.raiseError("Could not parse <collett:structure> section");
            } else if (xml.qualifiedName() == QStringLiteral("collett:content")) {
                if (!readContentXML(xml))
                    xml.raiseError("Could not parse <collett:content> section");
            } else if (xml.qualifiedName() == QStringLiteral("collett:extra")) {
                if (!readExtraXML(xml))
                    xml.raiseError("Could not parse <collett:extra> section");
            } else {
                XML_WARN_ELEM("Unknown", xml);
            }
            break;

        case QXmlStreamReader::EndElement:
            if (xml.qualifiedName() == QStringLiteral("collett:project")) {
                docStatus |= 0b1000;
            } else {
                XML_WARN_ELEM("Skipped", xml);
            }
            break;

        case QXmlStreamReader::Characters:
            if (!xml.isWhitespace()) {
                XML_WARN_GEN("Ignored content", xml);
            }
            break;

        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            XML_WARN_GEN("Ignored content", xml);
            break;

        default:
            XML_WARN_GEN("Unknown content", xml);
            break;
        }
    }
    if (xml.hasError()) {
        qWarning() << xml.errorString();
        return false;
    }

    return docStatus == 0b1111;
}

/**
 * Section Readers
 * ===============
 */

bool ProjectXmlReader::readMetaXML(QXmlStreamReader &xml) {

    Q_ASSERT(xml.isStartElement() && xml.qualifiedName() == QStringLiteral("collett:meta"));

    while (!xml.atEnd()) {
        switch(xml.readNext()) {

        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Invalid:
            XML_WARN_GEN("Invalid content", xml);
            break;

        case QXmlStreamReader::StartElement:
            XML_DEBUG("Element", xml);
            if (xml.qualifiedName() == QStringLiteral("dc:created")) {
                qDebug() << xml.readElementText();
            } else if (xml.qualifiedName() == QStringLiteral("dc:date")) {
                qDebug() << xml.readElementText();
            } else {
                XML_WARN_ELEM("Unknown", xml);
            }
            break;

        case QXmlStreamReader::EndElement:
            if (xml.qualifiedName() == QLatin1String("collett:meta")) {
                XML_DEBUG("Closing", xml);
                return true;
            } else {
                XML_WARN_ELEM("Skipped", xml);
            }
            break;

        case QXmlStreamReader::Characters:
            if (!xml.isWhitespace()) {
                XML_WARN_GEN("Ignored content", xml);
            }
            break;

        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            XML_WARN_GEN("Ignored content", xml);
            break;

        default:
            XML_WARN_GEN("Unknown content", xml);
            break;
        }
    }

    return false;
}

bool ProjectXmlReader::readSettingsXML(QXmlStreamReader &xml) {

    Q_ASSERT(xml.isStartElement() && xml.qualifiedName() == QStringLiteral("collett:settings"));

    while (!xml.atEnd()) {
        switch(xml.readNext()) {

        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Invalid:
            XML_WARN_GEN("Invalid content", xml);
            break;

        case QXmlStreamReader::StartElement:
            XML_DEBUG("Element", xml);
            XML_WARN_ELEM("Unknown", xml);
            break;

        case QXmlStreamReader::EndElement:
            if (xml.qualifiedName() == QLatin1String("collett:settings")) {
                XML_DEBUG("Closing", xml);
                return true;
            } else {
                XML_WARN_ELEM("Skipped", xml);
            }
            break;

        case QXmlStreamReader::Characters:
            if (!xml.isWhitespace()) {
                XML_WARN_GEN("Ignored content", xml);
            }
            break;

        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            XML_WARN_GEN("Ignored content", xml);
            break;

        default:
            XML_WARN_GEN("Unknown content", xml);
            break;
        }
    }

    return false;
}

bool ProjectXmlReader::readStylesXML(QXmlStreamReader &xml) {

    Q_ASSERT(xml.isStartElement() && xml.qualifiedName() == QStringLiteral("collett:styles"));

    while (!xml.atEnd()) {
        switch(xml.readNext()) {

        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Invalid:
            XML_WARN_GEN("Invalid content", xml);
            break;

        case QXmlStreamReader::StartElement:
            XML_DEBUG("Element", xml);
            XML_WARN_ELEM("Unknown", xml);
            break;

        case QXmlStreamReader::EndElement:
            if (xml.qualifiedName() == QLatin1String("collett:styles")) {
                XML_DEBUG("Closing", xml);
                return true;
            } else {
                XML_WARN_ELEM("Skipped", xml);
            }
            break;

        case QXmlStreamReader::Characters:
            if (!xml.isWhitespace()) {
                XML_WARN_GEN("Ignored content", xml);
            }
            break;

        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            XML_WARN_GEN("Ignored content", xml);
            break;

        default:
            XML_WARN_GEN("Unknown content", xml);
            break;
        }
    }

    return false;
}

bool ProjectXmlReader::readStructureXML(QXmlStreamReader &xml) {

    Q_ASSERT(xml.isStartElement() && xml.qualifiedName() == QStringLiteral("collett:structure"));

    while (!xml.atEnd()) {
        switch(xml.readNext()) {

        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Invalid:
            XML_WARN_GEN("Invalid content", xml);
            break;

        case QXmlStreamReader::StartElement:
            XML_DEBUG("Element", xml);
            XML_WARN_ELEM("Unknown", xml);
            break;

        case QXmlStreamReader::EndElement:
            if (xml.qualifiedName() == QLatin1String("collett:structure")) {
                XML_DEBUG("Closing", xml);
                return true;
            } else {
                XML_WARN_ELEM("Skipped", xml);
            }
            break;

        case QXmlStreamReader::Characters:
            if (!xml.isWhitespace()) {
                XML_WARN_GEN("Ignored content", xml);
            }
            break;

        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            XML_WARN_GEN("Ignored content", xml);
            break;

        default:
            XML_WARN_GEN("Unknown content", xml);
            break;
        }
    }

    return false;
}

bool ProjectXmlReader::readContentXML(QXmlStreamReader &xml) {

    Q_ASSERT(xml.isStartElement() && xml.qualifiedName() == QStringLiteral("collett:content"));

    while (!xml.atEnd()) {
        switch(xml.readNext()) {

        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Invalid:
            XML_WARN_GEN("Invalid content", xml);
            break;

        case QXmlStreamReader::StartElement:
            XML_DEBUG("Element", xml);
            XML_WARN_ELEM("Unknown", xml);
            break;

        case QXmlStreamReader::EndElement:
            if (xml.qualifiedName() == QLatin1String("collett:content")) {
                XML_DEBUG("Closing", xml);
                return true;
            } else {
                XML_WARN_ELEM("Skipped", xml);
            }
            break;

        case QXmlStreamReader::Characters:
            if (!xml.isWhitespace()) {
                XML_WARN_GEN("Ignored content", xml);
            }
            break;

        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            XML_WARN_GEN("Ignored content", xml);
            break;

        default:
            XML_WARN_GEN("Unknown content", xml);
            break;
        }
    }

    return false;
}

bool ProjectXmlReader::readExtraXML(QXmlStreamReader &xml) {

    Q_ASSERT(xml.isStartElement() && xml.qualifiedName() == QStringLiteral("collett:extra"));

    while (!xml.atEnd()) {
        switch(xml.readNext()) {

        case QXmlStreamReader::StartDocument:
        case QXmlStreamReader::EndDocument:
        case QXmlStreamReader::Invalid:
            XML_WARN_GEN("Invalid content", xml);
            break;

        case QXmlStreamReader::StartElement:
            XML_DEBUG("Element", xml);
            XML_WARN_ELEM("Unknown", xml);
            break;

        case QXmlStreamReader::EndElement:
            if (xml.qualifiedName() == QLatin1String("collett:extra")) {
                XML_DEBUG("Closing", xml);
                return true;
            } else {
                XML_WARN_ELEM("Skipped", xml);
            }
            break;

        case QXmlStreamReader::Characters:
            if (!xml.isWhitespace()) {
                XML_WARN_GEN("Ignored content", xml);
            }
            break;

        case QXmlStreamReader::Comment:
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            XML_WARN_GEN("Ignored content", xml);
            break;

        default:
            XML_WARN_GEN("Unknown content", xml);
            break;
        }
    }

    return false;
}


} // namespace Collett
