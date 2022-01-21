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

#include <stack>

#include <QUuid>
#include <QStack>
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
    m_docVersion = "";
    m_docEncoding = "";
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

    bool status = parseProlog(xml) && parseRoot(xml);

    if (!status) qWarning() << m_lastError;

    return status;
}

/**
 * Root Readers
 * ============
 */

bool ProjectXmlReader::parseProlog(QXmlStreamReader &xml) {

    while (!xml.atEnd()) {

        xml.readNext();
        if (xml.hasError()) {
            qWarning() << xml.errorString();
            m_lastError = xml.errorString();
            return false;
        }

        switch (xml.tokenType()) {
        case QXmlStreamReader::StartDocument:
            m_docVersion = xml.documentVersion().toString();
            m_docEncoding = xml.documentEncoding().toString();
            qDebug() << "XML Version:" << m_docVersion;
            qDebug() << "XML Encoding:" << m_docEncoding;
            break;

        case QXmlStreamReader::Comment:
            // Comments are ignored
            break;

        case QXmlStreamReader::DTD:
            xml.raiseError("The Collett XML parser did not expect a DTD element");
            return false;
            break;

        case QXmlStreamReader::ProcessingInstruction:
            xml.raiseError("The Collett XML parser did not expect a processing instruction");
            return false;
            break;

        default:
            // If the token is none of the above, prolog processing is done.
            return true;
        }
    }

    return true;
}

bool ProjectXmlReader::parseRoot(QXmlStreamReader &xml) {

    Q_ASSERT(xml.isStartElement());

    if (xml.qualifiedName() != QStringLiteral("collett:project")) {
        XML_WARN_ELEM("Unexpected", xml);
        return false;
    }

    QStack<QString> tagStack;

    while (!xml.atEnd() && !xml.hasError()) {

        switch (xml.tokenType()) {
        case QXmlStreamReader::StartElement:
            tagStack.push(xml.qualifiedName().toString());
            qDebug() << "Stack:" << tagStack;

            // Parse the element
            if (xml.qualifiedName() == QStringLiteral("collett:project")) {
                qInfo() << "Begin parsing project XML";

            } else if (xml.qualifiedName() == QStringLiteral("collett:meta")) {
                qDebug() << "Begin parsing project XML meta section";
                if (!parseValues(xml, tagStack, m_project->m_projectMeta))
                    xml.raiseError("Could not parse <collett:meta> section");

            } else if (xml.qualifiedName() == QStringLiteral("collett:settings")) {
                qDebug() << "Begin parsing project XML settings section";
                if (!parseValues(xml, tagStack, m_project->m_projectSettings))
                    xml.raiseError("Could not parse <collett:settings> section");

            } else if (xml.qualifiedName() == QStringLiteral("collett:styles")) {
            } else if (xml.qualifiedName() == QStringLiteral("collett:structure")) {
                qDebug() << "Begin parsing project XML structure section";
                if (!parseStructure(xml, tagStack))
                    xml.raiseError("Could not parse <collett:structure> section");

            } else if (xml.qualifiedName() == QStringLiteral("collett:content")) {
            } else if (xml.qualifiedName() == QStringLiteral("collett:extra")) {
            } else {
                XML_WARN_ELEM("Unexpected", xml);
            }
            break;

        case QXmlStreamReader::EndElement:
            if (tagStack.empty() || xml.qualifiedName() != tagStack.top()) {
                xml.raiseError("Unexpected end element");
                XML_WARN_ELEM("Unexpected", xml);
                return false;
            }
            tagStack.pop();
            qDebug() << "Stack:" << tagStack;
            break;

        case QXmlStreamReader::Characters:
            if (!xml.isWhitespace()) {
                if (!xml.text().toString().trimmed().isEmpty()) {
                    qDebug() << "Text:" << xml.text().toString();
                }
            }
            break;

        case QXmlStreamReader::Comment:
            // Comments are ignored
            break;

        default:
            xml.raiseError("Unexpected element");
            return false;
        }

        xml.readNext();
    }

    if (xml.hasError()) {
        qWarning() << xml.errorString();
        m_lastError = xml.errorString();
        xml.readNext();
        return false;
    }

    if (!tagStack.empty()) {
        m_lastError = "Tag mismatch";
        return false;
    }

    return true;
}

bool ProjectXmlReader::parseValues(QXmlStreamReader &xml, QStack<QString> &tagStack, QHash<QString, QString> &tagData) {

    Q_ASSERT(xml.isStartElement() && xml.qualifiedName() == tagStack.top());

    qsizetype inSize = tagStack.size();
    QString topTag = tagStack.top();

    while (!xml.atEnd() && !xml.hasError()) {

        xml.readNext();

        switch (xml.tokenType()) {
        case QXmlStreamReader::StartElement:
            tagStack.push(xml.qualifiedName().toString());
            qDebug() << "Stack:" << tagStack;
            break;

        case QXmlStreamReader::EndElement:
            if (tagStack.empty() || xml.qualifiedName() != tagStack.top()) {
                xml.raiseError("Unexpected end element");
                XML_WARN_ELEM("Unexpected", xml);
                return false;
            }
            if (tagStack.size() == inSize && tagStack.top() == topTag) {
                tagStack.pop();
                return true;
            }
            tagStack.pop();
            qDebug() << "Stack:" << tagStack;
            break;

        case QXmlStreamReader::Characters:
            if (!xml.isWhitespace()) {
                qDebug() << "Key:" << tagStack.top() << "Value:" << xml.text().toString();
                tagData.insert(tagStack.top(), xml.text().toString());
            }
            break;

        case QXmlStreamReader::Comment:
            // Comments are ignored
            break;

        default:
            xml.raiseError("Unexpected element");
            return false;
        }
    }

    return false;
}

/**
 * Section Readers
 * ===============
 */

bool ProjectXmlReader::parseStructure(QXmlStreamReader &xml, QStack<QString> &tagStack) {

    Q_ASSERT(xml.isStartElement() && xml.qualifiedName() == QStringLiteral("collett:structure"));

    qsizetype inSize = tagStack.size();

    bool inStory = false;

    while (!xml.atEnd()) {

        xml.readNext();

        switch (xml.tokenType()) {

        case QXmlStreamReader::StartElement:
            tagStack.push(xml.qualifiedName().toString());
            qDebug() << "Stack:" << tagStack;

            if (xml.qualifiedName() == QStringLiteral("collett:tree")) {
                QXmlStreamAttributes attr = xml.attributes();
                if (attr.value(QStringLiteral("item:class")) == QStringLiteral("story")) {
                    XML_DEBUG("Parsing", xml);
                    inStory = true;
                } else {
                    XML_WARN_ELEM("Unknown", xml);
                }
            } else if (xml.qualifiedName() == QStringLiteral("collett:item") && inStory) {
                if (!recurseStory(m_project->storyModel()->rootItem(), xml, tagStack))
                    xml.raiseError("Could not parse <collett:item> section");
            } else {
                XML_WARN_ELEM("Unknown", xml);
            }
            break;

        case QXmlStreamReader::EndElement:
            if (tagStack.empty() || xml.qualifiedName() != tagStack.top()) {
                xml.raiseError("Unexpected end element");
                XML_WARN_ELEM("Unexpected", xml);
                return false;
            }
            if (tagStack.size() == inSize && tagStack.top() == QStringLiteral("collett:structure")) {
                tagStack.pop();
                return true;
            }
            if (tagStack.top() == QStringLiteral("collett:tree")) {
                inStory = false;
            }
            tagStack.pop();
            qDebug() << "Stack:" << tagStack;
            break;

        case QXmlStreamReader::Characters:
            if (!xml.isWhitespace()) XML_WARN_GEN("Ignored", xml);
            break;

        case QXmlStreamReader::Comment:
            // Comments are ignored
            break;

        default:
            xml.raiseError("Unexpected element");
            return false;
        }
    }

    return false;
}

/**
 * Additional Reader Functions
 * ===========================
 */

bool ProjectXmlReader::recurseStory(StoryItem *parent, QXmlStreamReader &xml, QStack<QString> &tagStack) {

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

        xml.readNext();

        switch (xml.tokenType()) {

        case QXmlStreamReader::StartElement:
            tagStack.push(xml.qualifiedName().toString());
            qDebug() << "Stack:" << tagStack;

            if (xml.qualifiedName() == QStringLiteral("collett:item")) {
                XML_DEBUG("Parsing", xml);
                if (!recurseStory(item, xml, tagStack))
                    xml.raiseError("Could not parse <collett:item> section");
            } else {
                XML_WARN_ELEM("Unknown", xml);
            }
            break;

        case QXmlStreamReader::EndElement:
            if (tagStack.empty() || xml.qualifiedName() != tagStack.top()) {
                xml.raiseError("Unexpected end element");
                XML_WARN_ELEM("Unexpected", xml);
                return false;
            }
            tagStack.pop();
            qDebug() << "Stack:" << tagStack;
            if (xml.qualifiedName() == QStringLiteral("collett:item")) {
                XML_DEBUG("Closing", xml);
                return true;
            } else {
                XML_WARN_ELEM("Skipped", xml);
            }
            break;

        case QXmlStreamReader::Characters:
            if (!xml.isWhitespace()) XML_WARN_GEN("Ignored", xml);
            break;

        case QXmlStreamReader::Comment:
            // Comments are ignored
            break;

        default:
            xml.raiseError("Unexpected element");
            return false;
        }
    }

    return false;
}

} // namespace Collett
