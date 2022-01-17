/*
** Collett – Project XML Writer Class
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
#include "storyitem.h"
#include "projectxmlreader.h"
#include "projectxmlwriter.h"

#include <QString>
#include <QDateTime>
#include <QFileInfo>
#include <QApplication>

namespace Collett {

ProjectXmlWriter::ProjectXmlWriter(Project *project)
    : m_project(project)
{
    m_lastError = "";
}

ProjectXmlWriter::~ProjectXmlWriter() {
    qDebug() << "Destructor: ProjectXmlWriter";
}

bool ProjectXmlWriter::writeProjectFile() {

    QString filePath = m_project->projectPath();
    QFileInfo fInfo(filePath);

    if (fInfo.isDir()) {
        m_lastError = tr("Could not write project file. Path is a folder: %1").arg(filePath);
        qWarning() << "Could not write project file. Path is a folder:" << filePath;
        return false;
    }

    if (!fInfo.isWritable()) {
        m_lastError = tr("Could not write project file. Path is not writable: %1").arg(filePath);
        qWarning() << "Could not write project file. Path is not writable:" << filePath;
        return false;
    }

    QFile outFile(filePath);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        m_lastError = tr("Could not write project file: %1").arg(filePath);
        qWarning() << "Could not write project file:" << filePath;
        return false;
    }

    qDebug() << "Writing project to:" << filePath;

    QXmlStreamWriter xml(&outFile);
    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(2);
    xml.writeStartDocument();
    xml.writeNamespace(Collett::ColNsCollett, "collett");
    xml.writeNamespace(Collett::ColNsConfig, "config");
    xml.writeNamespace(Collett::ColNsItem, "item");
    xml.writeNamespace(Collett::ColNsMeta, "meta");
    xml.writeNamespace(Collett::ColNsStyle, "style");
    xml.writeNamespace(Collett::ColNsText, "text");
    xml.writeNamespace(Collett::XmlNsDC, "dc");

    xml.writeStartElement(Collett::ColNsCollett, "project");
    xml.writeAttribute(Collett::ColNsMeta, "file-version", "1.0-draft");
    xml.writeAttribute(Collett::ColNsMeta, "app-version", qApp->applicationVersion());

    // Write Data

    writeMetaXML(xml);
    writeSettingsXML(xml);
    writeStylesXML(xml);
    writeStructureXML(xml);
    writeContentXML(xml);
    writeExtraXML(xml);

    // Close XML File

    xml.writeEndElement(); // project
    xml.writeEndDocument();
    outFile.close();

    return true;
}

/**
 * Section Writers
 * ===============
 */

void ProjectXmlWriter::writeMetaXML(QXmlStreamWriter &xml) {

    xml.writeStartElement(Collett::ColNsCollett, QStringLiteral("meta"));

    xml.writeStartElement(Collett::XmlNsDC, QStringLiteral("created"));
    xml.writeCharacters(m_project->createdTime());
    xml.writeEndElement();

    xml.writeStartElement(Collett::XmlNsDC, QStringLiteral("date"));
    xml.writeCharacters(QDateTime::currentDateTime().toString(Qt::ISODate));
    xml.writeEndElement();

    xml.writeEndElement(); // meta

    return;
}

void ProjectXmlWriter::writeSettingsXML(QXmlStreamWriter &xml) {

    xml.writeStartElement(Collett::ColNsCollett, QStringLiteral("settings"));
    xml.writeEndElement(); // settings

    return;
}

void ProjectXmlWriter::writeStylesXML(QXmlStreamWriter &xml) {

    xml.writeStartElement(Collett::ColNsCollett, QStringLiteral("styles"));
    xml.writeEndElement(); // styles

    return;
}

void ProjectXmlWriter::writeStructureXML(QXmlStreamWriter &xml) {

    xml.writeStartElement(Collett::ColNsCollett, QStringLiteral("structure"));

    StoryItem *item = m_project->storyModel()->rootItem();
    if (item) {
        recurseStory(item, xml);
    }

    xml.writeEndElement(); // structure

    return;
}

void ProjectXmlWriter::writeContentXML(QXmlStreamWriter &xml) {

    xml.writeStartElement(Collett::ColNsCollett, QStringLiteral("content"));
    xml.writeEndElement(); // content

    return;
}

void ProjectXmlWriter::writeExtraXML(QXmlStreamWriter &xml) {

    xml.writeStartElement(Collett::ColNsCollett, QStringLiteral("extra"));
    xml.writeEndElement(); // extra

    return;
}

/**
 * Additional Writer Functions
 * ===========================
 */

void ProjectXmlWriter::recurseStory(StoryItem *item, QXmlStreamWriter &xml) {

    if (!item->m_parentItem) {
        xml.writeStartElement(Collett::ColNsCollett, QStringLiteral("tree"));
        xml.writeAttribute(Collett::ColNsItem, QStringLiteral("class"), QStringLiteral("story"));

    } else {
        xml.writeStartElement(Collett::ColNsCollett, QStringLiteral("item"));

        switch (item->m_type) {
            case StoryItem::Root:
                xml.writeAttribute(Collett::ColNsItem, QStringLiteral("type"), QStringLiteral("root"));
                break;
            case StoryItem::Book:
                xml.writeAttribute(Collett::ColNsItem, QStringLiteral("type"), QStringLiteral("book"));
                break;
            case StoryItem::Partition:
                xml.writeAttribute(Collett::ColNsItem, QStringLiteral("type"), QStringLiteral("partition"));
                break;
            case StoryItem::Chapter:
                xml.writeAttribute(Collett::ColNsItem, QStringLiteral("type"), QStringLiteral("chapter"));
                break;
            case StoryItem::Scene:
                xml.writeAttribute(Collett::ColNsItem, QStringLiteral("type"), QStringLiteral("scene"));
                break;
            case StoryItem::Page:
                xml.writeAttribute(Collett::ColNsItem, QStringLiteral("type"), QStringLiteral("page"));
                break;
            default:
                return;
                break;
        }

        xml.writeAttribute(Collett::ColNsItem, QStringLiteral("name"), item->m_name);
        xml.writeAttribute(Collett::ColNsItem, QStringLiteral("handle"), item->m_handle.toString(QUuid::WithoutBraces));
        xml.writeAttribute(Collett::ColNsItem, QStringLiteral("order"), QString().setNum(item->row()));
        xml.writeAttribute(Collett::ColNsItem, QStringLiteral("words"), QString().setNum(item->m_wCount));
    }

    for (qsizetype i=0; i<item->m_childItems.size(); ++i) {
        recurseStory(item->m_childItems.at(i), xml);
    }

    xml.writeEndElement(); // root or item
}

} // namespace Collett
