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

#include <QUuid>
#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>

#ifdef DEBUG
#define XML_DEBUG(m, x)(qDebug("%s <%s> at %d:%d", m, qPrintable(x.qualifiedName().toString()), int(x.lineNumber()), int(x.columnNumber())))
#else
#define XML_DEBUG(m, x)
#endif
#define XML_WARN_GEN(m, x)(qWarning("%s data at %d:%d", m, int(x.lineNumber()), int(x.columnNumber())))
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

        case QXmlStreamReader::StartDocument:
            docStatus |= 0b0001;
            break;

        case QXmlStreamReader::EndDocument:
            docStatus |= 0b0010;
            break;

        case QXmlStreamReader::StartElement:
            XML_DEBUG("Parsing", xml);
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
            if (!xml.isWhitespace()) XML_WARN_GEN("Ignored", xml);
            break;

        case QXmlStreamReader::Invalid:
            XML_WARN_GEN("Invalid", xml);
            break;

        default:
            XML_WARN_GEN("Ignored", xml);
            break;
        }
    }

    if (xml.hasError()) {
        qWarning() << xml.errorString();
        return false;
    }
    m_project->m_isValid = docStatus == 0b1111;

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

        case QXmlStreamReader::StartElement:
            if (xml.qualifiedName() == QStringLiteral("dc:created")) {
                XML_DEBUG("Parsing", xml);
                qDebug() << xml.readElementText();
            } else if (xml.qualifiedName() == QStringLiteral("dc:date")) {
                XML_DEBUG("Parsing", xml);
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
            if (!xml.isWhitespace()) XML_WARN_GEN("Ignored", xml);
            break;

        case QXmlStreamReader::Invalid:
            XML_WARN_GEN("Invalid", xml);
            break;

        default:
            XML_WARN_GEN("Ignored", xml);
            break;
        }
    }

    return false;
}

bool ProjectXmlReader::readSettingsXML(QXmlStreamReader &xml) {

    Q_ASSERT(xml.isStartElement() && xml.qualifiedName() == QStringLiteral("collett:settings"));

    while (!xml.atEnd()) {
        switch(xml.readNext()) {

        case QXmlStreamReader::StartElement:
            XML_DEBUG("Parsing", xml);
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
            if (!xml.isWhitespace()) XML_WARN_GEN("Ignored", xml);
            break;

        case QXmlStreamReader::Invalid:
            XML_WARN_GEN("Invalid", xml);
            break;

        default:
            XML_WARN_GEN("Ignored", xml);
            break;
        }
    }

    return false;
}

bool ProjectXmlReader::readStylesXML(QXmlStreamReader &xml) {

    Q_ASSERT(xml.isStartElement() && xml.qualifiedName() == QStringLiteral("collett:styles"));

    while (!xml.atEnd()) {
        switch(xml.readNext()) {

        case QXmlStreamReader::StartElement:
            XML_DEBUG("Parsing", xml);
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
            if (!xml.isWhitespace()) XML_WARN_GEN("Ignored", xml);
            break;

        case QXmlStreamReader::Invalid:
            XML_WARN_GEN("Invalid", xml);
            break;

        default:
            XML_WARN_GEN("Ignored", xml);
            break;
        }
    }

    return false;
}

bool ProjectXmlReader::readStructureXML(QXmlStreamReader &xml) {

    Q_ASSERT(xml.isStartElement() && xml.qualifiedName() == QStringLiteral("collett:structure"));

    while (!xml.atEnd()) {
        switch(xml.readNext()) {

        case QXmlStreamReader::StartElement:
            if (xml.qualifiedName() == QStringLiteral("collett:tree")) {
                QXmlStreamAttributes attr = xml.attributes();
                if (attr.value(QStringLiteral("item:class")) == QStringLiteral("story")) {
                    XML_DEBUG("Parsing", xml);
                    if (!readStoryTreeXML(xml))
                        xml.raiseError("Could not parse <collett:tree> section");
                } else {
                    XML_WARN_ELEM("Unknown", xml);
                }
            } else {
                XML_WARN_ELEM("Unknown", xml);
            }
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
            if (!xml.isWhitespace()) XML_WARN_GEN("Ignored", xml);
            break;

        case QXmlStreamReader::Invalid:
            XML_WARN_GEN("Invalid", xml);
            break;

        default:
            XML_WARN_GEN("Ignored", xml);
            break;
        }
    }

    return false;
}

bool ProjectXmlReader::readContentXML(QXmlStreamReader &xml) {

    Q_ASSERT(xml.isStartElement() && xml.qualifiedName() == QStringLiteral("collett:content"));

    while (!xml.atEnd()) {
        switch(xml.readNext()) {

        case QXmlStreamReader::StartElement:
            XML_DEBUG("Parsing", xml);
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
            if (!xml.isWhitespace()) XML_WARN_GEN("Ignored", xml);
            break;

        case QXmlStreamReader::Invalid:
            XML_WARN_GEN("Invalid", xml);
            break;

        default:
            XML_WARN_GEN("Ignored", xml);
            break;
        }
    }

    return false;
}

bool ProjectXmlReader::readExtraXML(QXmlStreamReader &xml) {

    Q_ASSERT(xml.isStartElement() && xml.qualifiedName() == QStringLiteral("collett:extra"));

    while (!xml.atEnd()) {
        switch(xml.readNext()) {

        case QXmlStreamReader::StartElement:
            XML_DEBUG("Parsing", xml);
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
            if (!xml.isWhitespace()) XML_WARN_GEN("Ignored", xml);
            break;

        case QXmlStreamReader::Invalid:
            XML_WARN_GEN("Invalid", xml);
            break;

        default:
            XML_WARN_GEN("Ignored", xml);
            break;
        }
    }

    return false;
}

/**
 * Additional Reader Functions
 * ===========================
 */

bool ProjectXmlReader::readStoryTreeXML(QXmlStreamReader &xml) {

    Q_ASSERT(xml.isStartElement() && xml.qualifiedName() == QStringLiteral("collett:tree"));

    while (!xml.atEnd()) {
        switch(xml.readNext()) {

        case QXmlStreamReader::StartElement:
            if (xml.qualifiedName() == QLatin1String("collett:item")) {
                XML_DEBUG("Parsing", xml);
                if (!recurseStory(m_project->storyModel()->rootItem(), xml))
                    xml.raiseError("Could not parse <collett:item> section");
            } else {
                XML_WARN_ELEM("Unknown", xml);
            }
            break;

        case QXmlStreamReader::EndElement:
            if (xml.qualifiedName() == QLatin1String("collett:tree")) {
                XML_DEBUG("Closing", xml);
                return true;
            } else {
                XML_WARN_ELEM("Skipped", xml);
            }
            break;

        case QXmlStreamReader::Characters:
            if (!xml.isWhitespace()) XML_WARN_GEN("Ignored", xml);
            break;

        case QXmlStreamReader::Invalid:
            XML_WARN_GEN("Invalid", xml);
            break;

        default:
            XML_WARN_GEN("Ignored", xml);
            break;
        }
    }

    return false;
}

bool ProjectXmlReader::recurseStory(StoryItem *parent, QXmlStreamReader &xml) {

    Q_ASSERT(xml.isStartElement() && xml.qualifiedName() == QStringLiteral("collett:item"));

    QXmlStreamAttributes attr = xml.attributes();
    StoryItem::ItemType type = StoryItem::typeFromString(attr.value(QStringLiteral("item:type")).toString());
    QString name = attr.value(QStringLiteral("item:name")).toString().simplified();
    QUuid handle = QUuid(attr.value(QStringLiteral("item:handle")).toString());
    int order = attr.value(QStringLiteral("item:order")).toInt();
    int words = attr.value(QStringLiteral("item:words")).toInt();

    if (type == StoryItem::Invalid) {
        xml.raiseError(QString("Invalid item type on line %1").arg(xml.lineNumber()));
        return false;
    }
    if (handle.isNull()) {
        xml.raiseError(QString("Invalid item handle on line %1").arg(xml.lineNumber()));
        return false;
    }

    StoryItem *item = parent->addChild(handle, name, type, words);
    if (!item) {
        xml.raiseError(QString("Failed to add item on line %1").arg(xml.lineNumber()));
        return false;
    }
    if (item->row() != order) {
        qWarning() << "Order mismatch for" << handle.toString(QUuid::WithoutBraces)
                   << "between XML and data model" << item->row() << "!=" << order;
    }

    qDebug() << "Loaded item" << attr.value(QStringLiteral("item:handle")).toString() << handle.toString(QUuid::WithoutBraces);

    while (!xml.atEnd()) {
        switch(xml.readNext()) {

        case QXmlStreamReader::StartElement:
            if (xml.qualifiedName() == QLatin1String("collett:item")) {
                XML_DEBUG("Parsing", xml);
                if (!recurseStory(item, xml))
                    xml.raiseError("Could not parse <collett:item> section");
            } else {
                XML_WARN_ELEM("Unknown", xml);
            }
            break;

        case QXmlStreamReader::EndElement:
            if (xml.qualifiedName() == QLatin1String("collett:item")) {
                XML_DEBUG("Closing", xml);
                return true;
            } else {
                XML_WARN_ELEM("Skipped", xml);
            }
            break;

        case QXmlStreamReader::Characters:
            if (!xml.isWhitespace()) XML_WARN_GEN("Ignored", xml);
            break;

        case QXmlStreamReader::Invalid:
            XML_WARN_GEN("Invalid", xml);
            break;

        default:
            XML_WARN_GEN("Ignored", xml);
            break;
        }
    }

    return false;
}

} // namespace Collett
