/*
** Collett – Icons Class
** =====================
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

#include "collett.h"
#include "icons.h"
#include "settings.h"
#include "theme.h"

#include <QByteArray>
#include <QFileInfo>
#include <QIcon>
#include <QPixmap>
#include <QSize>
#include <QString>
#include <QTextStream>

namespace Collett {

// Constructor/Destructor
// ======================

Icons::Icons(Theme *parent) : QObject(parent), m_theme(parent) {
    m_settings = Settings::instance();
}

Icons::~Icons() {
    qDebug() << "Destructor: Icons";
}

// Getters
// =======

QIcon Icons::getIcon(QString name, ThemeColor color, QSize size) {
    QString key = name + QString::number(color) + "-"
                + QString::number(size.width()) + "x"
                + QString::number(size.height());
    if (!m_icons.contains(key)) {
        m_icons[key] = this->generateIcon(name, color, size);
    }
    qDebug() << "Requested Icon:" << key;
    return m_icons[key];
}

QIcon Icons::getProjectIcon(ItemType itemType, ItemClass itemClass, ItemLevel itemLevel, QSize size) {
    QString name = "none";
    ThemeColor color = ThemeColor::DefaultColor;
    switch (itemType) {
        case ItemType::RootType:
            switch (itemClass) {
                case ItemClass::NovelClass:
                    name = "cls_novel";
                    break;
                case ItemClass::CharacterClass:
                    name = "cls_character";
                    break;
                case ItemClass::PlotClass:
                    name = "cls_plot";
                    break;
                case ItemClass::LocationClass:
                    name = "cls_location";
                    break;
                case ItemClass::ObjectClass:
                    name = "cls_object";
                    break;
                case ItemClass::EntityClass:
                    name = "cls_entity";
                    break;
                case ItemClass::CustomClass:
                    name = "cls_custom";
                    break;
                case ItemClass::ArchiveClass:
                    name = "cls_archive";
                    break;
                case ItemClass::TrashClass:
                    name = "cls_trash";
                    break;
            }
            color = ThemeColor::RootColor;
            break;
        case ItemType::FolderType:
            name = "prj_folder";
            color = ThemeColor::FolderColor;
            break;
        case ItemType::FileType:
            switch (itemLevel) {
                case ItemLevel::NoteLevel:
                    name = "prj_note";
                    color = ThemeColor::NoteColor;
                    break;
                case ItemLevel::TitleLevel:
                    name = "prj_title";
                    color = ThemeColor::TitleColor;
                    break;
                case ItemLevel::ChapterLevel:
                    name = "prj_chapter";
                    color = ThemeColor::ChapterColor;
                    break;
                case ItemLevel::SceneLevel:
                    name = "prj_scene";
                    color = ThemeColor::SceneColor;
                    break;
                default:
                    name = "prj_document";
                    color = ThemeColor::FileColor;
                    break;
            }
        default:
            break;
    }
    if (name != "none") {
        return this->getIcon(name, color, size);
    } else {
        return QIcon();
    }
}

// Public Methods
// ==============

bool Icons::loadIcons(QString icons) {

    QFileInfo iconsFile = QFileInfo(Settings::assetPath("icons").filePath(icons + ".icons"));
    if (!iconsFile.exists()) return false;
    qInfo() << "Loading Icons:" << icons;

    QFile file(iconsFile.filePath());
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Could not open file:" << iconsFile.filePath();
        return false;
    }

    QTextStream input(&file);
    while (!input.atEnd()) {
        QString line = input.readLine();
        qsizetype eqPos = line.indexOf("=");
        if (eqPos > 5) {
            if (line.startsWith("icon:")) {
                QString key(line.first(eqPos).sliced(5).trimmed());
                QByteArray svg(line.sliced(eqPos + 1).trimmed().toLatin1());
                if (svg.startsWith("<svg")) m_svg[key] = svg;
            } else if (line.startsWith("meta:name")) {
                m_name = line.sliced(eqPos + 1).trimmed();
                qDebug() << "IconSet Name:" << m_name;
            } else if (line.startsWith("meta:author")) {
                m_author = line.sliced(eqPos + 1).trimmed();
                qDebug() << "IconSet Author:" << m_author;
            } else if (line.startsWith("meta:license")) {
                m_license = line.sliced(eqPos + 1).trimmed();
                qDebug() << "IconSet License:" << m_license;
            }
        }
    }
    file.close();

    return true;
}

// Private Methods
// ===============

QIcon Icons::generateIcon(QString name, ThemeColor color, QSize size) {
    if (m_svg.contains(name)) {
        QByteArray svg(m_svg[name]);
        svg.replace("#000000", QByteArray::fromStdString(m_theme->m_colors.at(color).name(QColor::HexRgb).toStdString()));
        QPixmap pixmap(size);
        pixmap.fill(Qt::transparent);
        pixmap.loadFromData(svg, "svg");
        return QIcon(pixmap);
    }
    return QIcon();
}

} // namespace Collett
