/*
** Collett – Project Class
** =======================
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
#include "project.h"
#include "storage.h"
#include "storymodel.h"

#include <QDir>
#include <QFile>
#include <QString>
#include <QDateTime>
#include <QFileInfo>
#include <QIODevice>
#include <QByteArray>
#include <QTextStream>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QXmlStreamWriter>

namespace Collett {

/**
 * Class Constructor/Destructor
 * ============================
 */

Project::Project(const QString &path) {

    m_isValid = false;
    m_storyModel = new StoryModel(this);
    m_createdTime = QDateTime::currentDateTime().toString(Qt::ISODate);

    // If the path is a file, go one level up
    QFileInfo fObj(path);
    if (fObj.isFile()) {
        m_store = new Storage(fObj.dir().path(), Storage::Folder);
    } else {
        m_store = new Storage(path, Storage::Folder);
    }

    qDebug() << "Project Path:" << m_store->projectPath();
}

Project::~Project() {
    qDebug() << "Destructor: Project";
    delete m_storyModel;
}

/**
 * Class Methods
 * =============
 */

bool Project::openProject() {

    qInfo() << "Loading Project:" << m_store->projectPath();
    if (!m_store->isValid()) {
        qWarning() << "Cannot load project from this path";
        return false;
    }

    bool main = m_store->loadProjectFile();
    if (main) {
        bool settings = loadSettingsFile();
        bool story = loadStoryFile();
        m_isValid = settings & story;
    } else {
        m_isValid = false;
    }

    return m_isValid;
}

bool Project::saveProject() {

    qInfo() << "Saving Project:" << m_store->projectPath();
    if (!m_isValid) {
        qWarning() << "No project open, nothing to save";
        return false;
    }

    // Open XML File

    QDir projPath(m_store->projectPath());
    QString projFile = projPath.filePath(m_projectFile);
    QFile outFile(projFile);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        m_lastError = tr("Could not write project file: %1").arg(projFile);
        qWarning() << "Could not write project file:" << projFile;
        return false;
    }

    QXmlStreamWriter xmlWriter(&outFile);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.setAutoFormattingIndent(2);
    xmlWriter.writeStartDocument();
    xmlWriter.writeNamespace(Collett::ColNsCollett, "collett");
    xmlWriter.writeNamespace(Collett::ColNsConfig, "config");
    xmlWriter.writeNamespace(Collett::ColNsItem, "item");
    xmlWriter.writeNamespace(Collett::ColNsMeta, "meta");
    xmlWriter.writeNamespace(Collett::ColNsStyle, "style");
    xmlWriter.writeNamespace(Collett::ColNsText, "text");
    xmlWriter.writeNamespace(Collett::XmlNsDC, "dc");
    xmlWriter.writeStartElement(Collett::ColNsCollett, "project");
    xmlWriter.writeAttribute(Collett::ColNsMeta, "file-version", "1.0-draft");
    xmlWriter.writeAttribute(Collett::ColNsMeta, "app-version", qApp->applicationVersion());

    // Write Data

    writeMetaXML(xmlWriter);
    writeSettingsXML(xmlWriter);
    writeStylesXML(xmlWriter);
    writeStructureXML(xmlWriter);
    writeContentXML(xmlWriter);
    writeExtraXML(xmlWriter);

    // Close XML File

    xmlWriter.writeEndElement(); // project
    xmlWriter.writeEndDocument();
    outFile.close();

    return true;
}

bool Project::isValid() const {
    return m_isValid;
}

/**
 * Class Setters
 * =============
 */

void Project::setProjectName(const QString &name) {
    m_projectName = name.simplified();
}

/**
 * Class Getters
 * =============
 */

QString Project::projectName() const {
    return m_projectName;
}

StoryModel *Project::storyModel() {
    return m_storyModel;
}

/**
 * Settings File
 * =============
 * Load and save functions for the project/project.json file.
 *
 * This file contains all the meta data and options for the Collett project,
 * except for the project content itself (the documents).
 */

bool Project::loadSettingsFile() {

    QJsonObject jData;
    if (!m_store->loadFile("project", jData)) {
        m_lastError = m_store->lastError();
        return false;
    }

    QJsonObject jMeta = jData["meta"].toObject();
    QJsonObject jProject = jData["project"].toObject();
    QJsonObject jSettings = jData["settings"].toObject();

    m_createdTime = Storage::getJsonString(jMeta, "created", "Unknown");
    m_projectName = Storage::getJsonString(jProject, "projectName", tr("Unnamed Project"));

    return true;
}

/**
 * Story File
 * ==========
 * Load and save functions for the project/story.json file.
 *
 * This file contains the structure of StoryItems contained in the StoryModel.
 * The structure is contained as child items under a single root item, and is
 * saved to a QJsonDocument by recursively calling the StoryItem->toJsonObject
 * function.
`*/

bool Project::loadStoryFile() {
    QJsonObject jData;
    if (!m_store->loadFile("story", jData)) {
        m_lastError = m_store->lastError();
        return false;
    }
    return m_storyModel->fromJsonObject(jData);
}

/**
 * XML Writers
 * ===========
 */

void Project::writeMetaXML(QXmlStreamWriter &xmlWriter) {

    xmlWriter.writeStartElement(Collett::ColNsCollett, "meta");

    xmlWriter.writeStartElement(Collett::XmlNsDC, "created");
    xmlWriter.writeCharacters(m_createdTime);
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement(Collett::XmlNsDC, "date");
    xmlWriter.writeCharacters(QDateTime::currentDateTime().toString(Qt::ISODate));
    xmlWriter.writeEndElement();

    xmlWriter.writeEndElement(); // meta

    return;
}

void Project::writeSettingsXML(QXmlStreamWriter &xmlWriter) {

    xmlWriter.writeStartElement(Collett::ColNsCollett, "settings");
    xmlWriter.writeEndElement(); // settings

    return;
}

void Project::writeStylesXML(QXmlStreamWriter &xmlWriter) {

    xmlWriter.writeStartElement(Collett::ColNsCollett, "styles");
    xmlWriter.writeEndElement(); // styles

    return;
}

void Project::writeStructureXML(QXmlStreamWriter &xmlWriter) {

    xmlWriter.writeStartElement(Collett::ColNsCollett, "structure");

    m_storyModel->writeXML(xmlWriter);

    xmlWriter.writeEndElement(); // structure

    return;
}

void Project::writeContentXML(QXmlStreamWriter &xmlWriter) {

    xmlWriter.writeStartElement(Collett::ColNsCollett, "content");

    xmlWriter.writeStartElement(Collett::ColNsCollett, "story");
    xmlWriter.writeEndElement(); // story

    xmlWriter.writeStartElement(Collett::ColNsCollett, "notes");
    xmlWriter.writeEndElement(); // notes

    xmlWriter.writeEndElement(); // content

    return;
}

void Project::writeExtraXML(QXmlStreamWriter &xmlWriter) {

    xmlWriter.writeStartElement(Collett::ColNsCollett, "extra");
    xmlWriter.writeEndElement(); // extra

    return;
}

/**
 * Error Handling
 * ==============
 */

bool Project::hasError() const {
    return !m_lastError.isEmpty();
}

QString Project::lastError() const {
    return m_lastError;
}

} // namespace Collett
