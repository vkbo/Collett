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

#ifndef COLLETT_PROJECT_XML_READER_H
#define COLLETT_PROJECT_XML_READER_H

#include "collett.h"
#include "project.h"

#include <QObject>
#include <QString>
#include <QXmlStreamReader>

namespace Collett {

class ProjectXmlReader : public QObject
{
    Q_OBJECT

public:
    ProjectXmlReader(Project *project);
    ~ProjectXmlReader();

    bool readProjectFile();

private:
    Project *m_project;
    QString m_lastError;

    bool readMetaXML(QXmlStreamReader &xml);
    bool readSettingsXML(QXmlStreamReader &xml);
    bool readStylesXML(QXmlStreamReader &xml);
    bool readStructureXML(QXmlStreamReader &xml);
    bool readContentXML(QXmlStreamReader &xml);
    bool readExtraXML(QXmlStreamReader &xml);

    bool readStoryTreeXML(QXmlStreamReader &xml);
    bool recurseStory(StoryItem *parent, QXmlStreamReader &xml);

};
} // namespace Collett

#endif // COLLETT_PROJECT_XML_READER_H
