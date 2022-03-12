/*
** Collett – Core Icon Repository
** ==============================
**
** This file is a part of Collett
** Copyright 2021–2022, Veronica Berglyd Olsen
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

#include "icons.h"
#include "svgiconengine.h"

#include <QIcon>
#include <QColor>
#include <QDebug>
#include <QString>
#include <QPalette>
#include <QApplication>

namespace Collett {

CollettIcons *CollettIcons::staticInstance = nullptr;
CollettIcons *CollettIcons::instance() {
    if (staticInstance == nullptr) {
        staticInstance = new CollettIcons();
        qDebug() << "Constructor: CollettIcons (Static)";
    }
    return staticInstance;
}

void CollettIcons::destroy() {
    if (staticInstance != nullptr) {
        qDebug() << "Destructor: CollettIcons (Static)";
        delete CollettIcons::staticInstance;
    }
}

/**!
 * @brief Construct a CollettIcons object.
 *
 * The icon SVG data is from https://fonts.google.com/icons
 * Icon style: Outlined, Filled
 */
CollettIcons::CollettIcons() {

    // Default Style
    this->setIconStyle(qApp->palette().buttonText().color(), 0.7);

    // inventory_2_black_24dp.svg
    m_svgPath["sideCollection"] = QByteArray(
        "M20,2H4C3,2,2,2.9,2,4v3.01C2,7.73,2.43,8.35,3,8.7V20c0,1.1,1.1,2,2,2h14c0.9,0,2-0.9,2-2V8"
        ".7c0.57-0.35,1-0.97,1-1.69V4 C22,2.9,21,2,20,2z M19,20H5V9h14V20z M20,7H4V4h16V7z"
    );

    // snippet_folder_black_24dp.svg
    m_svgPath["sideExplore"] = QByteArray(
        "M20,6h-8l-2-2H4C2.9,4,2.01,4.9,2.01,6L2,18c0,1.1,0.9,2,2,2h16c1.1,0,2-0.9,2-2V8C22,6.9,21"
        ".1,6,20,6z M20,18L4,18V6h5.17 l2,2H20V18z M17.5,12.12v3.38l-3,0v-5h1.38L17.5,12.12z M13,9"
        "v8l6,0v-5.5L16.5,9H13z"
    );

    // settings_black_24dp.svg
    m_svgPath["sideSettings"] = QByteArray(
        "M19.43 12.98c.04-.32.07-.64.07-.98 0-.34-.03-.66-.07-.98l2.11-1.65c.19-.15.24-.42.12-.64l"
        "-2-3.46c-.09-.16-.26-.25-.44-.25-.06 0-.12.01-.17.03l-2.49 1c-.52-.4-1.08-.73-1.69-.98l-."
        "38-2.65C14.46 2.18 14.25 2 14 2h-4c-.25 0-.46.18-.49.42l-.38 2.65c-.61.25-1.17.59-1.69.98"
        "l-2.49-1c-.06-.02-.12-.03-.18-.03-.17 0-.34.09-.43.25l-2 3.46c-.13.22-.07.49.12.64l2.11 1"
        ".65c-.04.32-.07.65-.07.98 0 .33.03.66.07.98l-2.11 1.65c-.19.15-.24.42-.12.64l2 3.46c.09.1"
        "6.26.25.44.25.06 0 .12-.01.17-.03l2.49-1c.52.4 1.08.73 1.69.98l.38 2.65c.03.24.24.42.49.4"
        "2h4c.25 0 .46-.18.49-.42l.38-2.65c.61-.25 1.17-.59 1.69-.98l2.49 1c.06.02.12.03.18.03.17 "
        "0 .34-.09.43-.25l2-3.46c.12-.22.07-.49-.12-.64l-2.11-1.65zm-1.98-1.71c.04.31.05.52.05.73 "
        "0 .21-.02.43-.05.73l-.14 1.13.89.7 1.08.84-.7 1.21-1.27-.51-1.04-.42-.9.68c-.43.32-.84.56"
        "-1.25.73l-1.06.43-.16 1.13-.2 1.35h-1.4l-.19-1.35-.16-1.13-1.06-.43c-.43-.18-.83-.41-1.23"
        "-.71l-.91-.7-1.06.43-1.27.51-.7-1.21 1.08-.84.89-.7-.14-1.13c-.03-.31-.05-.54-.05-.74s.02"
        "-.43.05-.73l.14-1.13-.89-.7-1.08-.84.7-1.21 1.27.51 1.04.42.9-.68c.43-.32.84-.56 1.25-.73"
        "l1.06-.43.16-1.13.2-1.35h1.39l.19 1.35.16 1.13 1.06.43c.43.18.83.41 1.23.71l.91.7 1.06-.4"
        "3 1.27-.51.7 1.21-1.07.85-.89.7.14 1.13zM12 8c-2.21 0-4 1.79-4 4s1.79 4 4 4 4-1.79 4-4-1."
        "79-4-4-4zm0 6c-1.1 0-2-.9-2-2s.9-2 2-2 2 .9 2 2-.9 2-2 2z"
    );

    // Generic Icons
    // =============

    // chevron_left_black_24dp.svg
    m_svgPath["chevronLeft"] = QByteArray(
        "M15.41 7.41L14 6l-6 6 6 6 1.41-1.41L10.83 12l4.58-4.59z"
    );

    // chevron_right_black_24dp.svg
    m_svgPath["chevronRight"] = QByteArray(
        "M10 6L8.59 7.41 13.17 12l-4.58 4.59L10 18l6-6-6-6z"
    );

    // add_black_24dp.svg
    m_svgPath["add"] = QByteArray(
        "M19 13h-6v6h-2v-6H5v-2h6V5h2v6h6v2z"
    );

    // ------ OLD VERSION ------------------------------------------------------------------------ //

    // Generic Icons
    // =============

    // more_vert_black_24dp.svg
    m_svgPath["more"] = QByteArray(
        "M12 8c1.1 0 2-.9 2-2s-.9-2-2-2-2 .9-2 2 .9 2 2 2zm0 2c-1.1 0-2 .9-2 2s.9 2 2 2 2-.9 2-2-."
        "9-2-2-2zm0 6c-1.1 0-2 .9-2 2s.9 2 2 2 2-.9 2-2-.9-2-2-2z"
    );

    // settings_black_24dp.svg
    m_svgPath["settings"] = QByteArray(
        "M19.5,12c0-0.23-0.01-0.45-0.03-0.68l1.86-1.41c0.4-0.3,0.51-0.86,0.26-1.3l-1.87-3.23c-0.25"
        "-0.44-0.79-0.62-1.25-0.42 l-2.15,0.91c-0.37-0.26-0.76-0.49-1.17-0.68l-0.29-2.31C14.8,2.38"
        ",14.37,2,13.87,2h-3.73C9.63,2,9.2,2.38,9.14,2.88L8.85,5.19 c-0.41,0.19-0.8,0.42-1.17,0.68"
        "L5.53,4.96c-0.46-0.2-1-0.02-1.25,0.42L2.41,8.62c-0.25,0.44-0.14,0.99,0.26,1.3l1.86,1.41 C"
        "4.51,11.55,4.5,11.77,4.5,12s0.01,0.45,0.03,0.68l-1.86,1.41c-0.4,0.3-0.51,0.86-0.26,1.3l1."
        "87,3.23c0.25,0.44,0.79,0.62,1.25,0.42 l2.15-0.91c0.37,0.26,0.76,0.49,1.17,0.68l0.29,2.31C"
        "9.2,21.62,9.63,22,10.13,22h3.73c0.5,0,0.93-0.38,0.99-0.88l0.29-2.31 c0.41-0.19,0.8-0.42,1"
        ".17-0.68l2.15,0.91c0.46,0.2,1,0.02,1.25-0.42l1.87-3.23c0.25-0.44,0.14-0.99-0.26-1.3l-1.86"
        "-1.41 C19.49,12.45,19.5,12.23,19.5,12z M12.04,15.5c-1.93,0-3.5-1.57-3.5-3.5s1.57-3.5,3.5-"
        "3.5s3.5,1.57,3.5,3.5S13.97,15.5,12.04,15.5z"
    );

    // Main ToolBar
    // ============

    // inventory_2_black_24dp.svg
    m_svgPath["collectionMenu"] = QByteArray(
        "M20,2H4C3,2,2,2.9,2,4v3.01C2,7.73,2.43,8.35,3,8.7V20c0,1.1,1.1,2,2,2h14c0.9,0,2-0.9,2-2V8"
        ".7c0.57-0.35,1-0.97,1-1.69V4 C22,2.9,21,2,20,2z M15,14H9v-2h6V14z M20,7H4V4h16V7z"
    );

    // description_black_24dp.svg
    m_svgPath["documentMenu"] = QByteArray(
        "M14 2H6c-1.1 0-1.99.9-1.99 2L4 20c0 1.1.89 2 1.99 2H18c1.1 0 2-.9 2-2V8l-6-6zm2 16H8v-2h8"
        "v2zm0-4H8v-2h8v2zm-3-5V3.5L18.5 9H13z"
    );

    // Tree ToolBar
    // ============

    // auto_stories_black_24dp.svg
    m_svgPath["storyModel"] = QByteArray(
        "M19 1l-5 5v11l5-4.5V1zM1 6v14.65c0 .25.25.5.5.5.1 0 .15-.05.25-.05C3.1 20.45 5.05 20 6.5 "
        "20c1.95 0 4.05.4 5.5 1.5V6c-1.45-1.1-3.55-1.5-5.5-1.5S2.45 4.9 1 6zm22 13.5V6c-.6-.45-1.2"
        "5-.75-2-1v13.5c-1.1-.35-2.3-.5-3.5-.5-1.7 0-4.15.65-5.5 1.5v2c1.35-.85 3.8-1.5 5.5-1.5 1."
        "65 0 3.35.3 4.75 1.05.1.05.15.05.25.05.25 0 .5-.25.5-.5v-1.1z"
    );

    // account_tree_black_24dp.svg
    m_svgPath["plotModel"] = QByteArray(
        "M22,11V3h-7v3H9V3H2v8h7V8h2v10h4v3h7v-8h-7v3h-2V8h2v3H22z M7,9H4V5h3V9z M17,15h3v4h-3V15z"
        " M17,5h3v4h-3V5z"
    );

    // face_black_24dp.svg
    m_svgPath["characterModel"] = QByteArray(
        "M10.25 13c0 .69-.56 1.25-1.25 1.25S7.75 13.69 7.75 13s.56-1.25 1.25-1.25 1.25.56 1.25 1.2"
        "5zM15 11.75c-.69 0-1.25.56-1.25 1.25s.56 1.25 1.25 1.25 1.25-.56 1.25-1.25-.56-1.25-1.25-"
        "1.25zm7 .25c0 5.52-4.48 10-10 10S2 17.52 2 12 6.48 2 12 2s10 4.48 10 10zM10.66 4.12C12.06"
        " 6.44 14.6 8 17.5 8c.46 0 .91-.05 1.34-.12C17.44 5.56 14.9 4 12 4c-.46 0-.91.05-1.34.12zM"
        "4.42 9.47c1.71-.97 3.03-2.55 3.66-4.44C6.37 6 5.05 7.58 4.42 9.47zM20 12c0-.78-.12-1.53-."
        "33-2.24-.7.15-1.42.24-2.17.24-3.13 0-5.92-1.44-7.76-3.69C8.69 8.87 6.6 10.88 4 11.86c.01."
        "04 0 .09 0 .14 0 4.41 3.59 8 8 8s8-3.59 8-8z"
    );

    // place_black_24dp.svg
    m_svgPath["locationModel"] = QByteArray(
        "M12 12c-1.1 0-2-.9-2-2s.9-2 2-2 2 .9 2 2-.9 2-2 2zm6-1.8C18 6.57 15.35 4 12 4s-6 2.57-6 6"
        ".2c0 2.34 1.95 5.44 6 9.14 4.05-3.7 6-6.8 6-9.14zM12 2c4.2 0 8 3.22 8 8.2 0 3.32-2.67 7.2"
        "5-8 11.8-5.33-4.55-8-8.48-8-11.8C4 5.22 7.8 2 12 2z"
    );

    // star_outline_black_24dp.svg
    m_svgPath["genericModel"] = QByteArray(
        "M22 9.24l-7.19-.62L12 2 9.19 8.63 2 9.24l5.46 4.73L5.82 21 12 17.27 18.18 21l-1.63-7.03L2"
        "2 9.24zM12 15.4l-3.76 2.27 1-4.28-3.32-2.88 4.38-.38L12 6.1l1.71 4.04 4.38.38-3.32 2.88 1"
        " 4.28L12 15.4z"
    );

    // Text Formatting Icons
    // =====================

    // format_size_black_24dp.svg
    m_svgPath["formatText"] = QByteArray(
        "M9 4v3h5v12h3V7h5V4H9zm-6 8h3v7h3v-7h3V9H3v3z"
    );

    // format_bold_black_24dp.svg
    m_svgPath["formatBold"] = QByteArray(
        "M15.6 10.79c.97-.67 1.65-1.77 1.65-2.79 0-2.26-1.75-4-4-4H7v14h7.04c2.09 0 3.71-1.7 3.71-"
        "3.79 0-1.52-.86-2.82-2.15-3.42zM10 6.5h3c.83 0 1.5.67 1.5 1.5s-.67 1.5-1.5 1.5h-3v-3zm3.5"
        " 9H10v-3h3.5c.83 0 1.5.67 1.5 1.5s-.67 1.5-1.5 1.5z"
    );

    // format_italic_black_24dp.svg
    m_svgPath["formatItalic"] = QByteArray(
        "M10 4v3h2.21l-3.42 8H6v3h8v-3h-2.21l3.42-8H18V4z"
    );

    // format_underlined_black_24dp.svg
    m_svgPath["formatUnderline"] = QByteArray(
        "M12 17c3.31 0 6-2.69 6-6V3h-2.5v8c0 1.93-1.57 3.5-3.5 3.5S8.5 12.93 8.5 11V3H6v8c0 3.31 2"
        ".69 6 6 6zm-7 2v2h14v-2H5z"
    );

    // format_strikethrough_black_24dp.svg
    m_svgPath["formatStrikeOut"] = QByteArray(
        "M10 19h4v-3h-4v3zM5 4v3h5v3h4V7h5V4H5zM3 14h18v-2H3v2z"
    );

    // subscript_black_24dp.svg
    m_svgPath["formatSubScript"] = QByteArray(
        "M22,18h-2v1h3v1h-4v-2c0-0.55,0.45-1,1-1h2v-1h-3v-1h3c0.55,0,1,0.45,1,1v1C23,17.55,22.55,1"
        "8,22,18z M5.88,18h2.66 l3.4-5.42h0.12l3.4,5.42h2.66l-4.65-7.27L17.81,4h-2.68l-3.07,4.99h-"
        "0.12L8.85,4H6.19l4.32,6.73L5.88,18z"
    );

    // superscript_black_24dp.svg
    m_svgPath["formatSupScript"] = QByteArray(
        "M22,7h-2v1h3v1h-4V7c0-0.55,0.45-1,1-1h2V5h-3V4h3c0.55,0,1,0.45,1,1v1C23,6.55,22.55,7,22,7"
        "z M5.88,20h2.66l3.4-5.42h0.12 l3.4,5.42h2.66l-4.65-7.27L17.81,6h-2.68l-3.07,4.99h-0.12L8."
        "85,6H6.19l4.32,6.73L5.88,20z"
    );

    // format_align_center_black_24dp.svg
    m_svgPath["alignCentre"] = QByteArray(
        "M7 15v2h10v-2H7zm-4 6h18v-2H3v2zm0-8h18v-2H3v2zm4-6v2h10V7H7zM3 3v2h18V3H3z"
    );

    // format_align_justify_black_24dp.svg
    m_svgPath["alignJustify"] = QByteArray(
        "M3 21h18v-2H3v2zm0-4h18v-2H3v2zm0-4h18v-2H3v2zm0-4h18V7H3v2zm0-6v2h18V3H3z"
    );

    // format_align_left_black_24dp.svg
    m_svgPath["alignLeft"] = QByteArray(
        "M15 15H3v2h12v-2zm0-8H3v2h12V7zM3 13h18v-2H3v2zm0 8h18v-2H3v2zM3 3v2h18V3H3z"
    );

    // format_align_right_black_24dp.svg
    m_svgPath["alignRight"] = QByteArray(
        "M3 21h18v-2H3v2zm6-4h12v-2H9v2zm-6-4h18v-2H3v2zm6-4h12V7H9v2zM3 3v2h18V3H3z"
    );

    // subject_black_24dp.svg (rotated 180 degrees, short bar replaces middle bars)
    m_svgPath["textSegment"] = QByteArray(
        "m20 9h-10v2h10zm0 4h-10v2h10zm0 4h-10v2h10zm-16-12v2h16v-2z"
    );

    // subject_black_24dp.svg (rotated 180 degrees)
    m_svgPath["textIndent"] = QByteArray(
        "m10 7h10v-2h-10zm-6 8h16v-2h-16zm16-6h-16v2h16zm0 10v-2h-16v2z"
    );

    // format_indent_increase_black_24dp.svg
    m_svgPath["blockIndent"] = QByteArray(
        "M3 21h18v-2H3v2zM3 8v8l4-4-4-4zm8 9h10v-2H11v2zM3 3v2h18V3H3zm8 6h10V7H11v2zm0 4h10v-2H11"
        "v2z"
    );

    // format_indent_decrease_black_24dp.svg
    m_svgPath["blockOutdent"] = QByteArray(
        "M11 17h10v-2H11v2zm-8-5l4 4V8l-4 4zm0 9h18v-2H3v2zM3 3v2h18V3H3zm8 6h10V7H11v2zm0 4h10v-2"
        "H11v2z"
    );
}

CollettIcons::~CollettIcons() {
    qDebug() << "Destructor: CollettIcons";
}

void CollettIcons::setIconStyle(const QColor &foreground, qreal opacity) {
    m_svgTemplateA = QByteArray(
        "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' height='24px' width='24px' fill='"
    ).append(
        foreground.name(QColor::HexRgb).toLatin1()
    ).append(
        "' opacity='"
    ).append(
        QByteArray().setNum(opacity)
    ).append(
        "'><path d='M0 0h24v24H0z' fill='none'/><path d='"
    );
}

QIcon CollettIcons::icon(const QString &name) {
    return QIcon(
        new SVGIconEngine(
            QByteArray().append(m_svgTemplateA).append(m_svgPath[name]).append(m_svgTemplateB)
        )
    );
}

bool CollettIcons::contains(const QString &name) {
    return m_svgPath.contains(name);
}

} // namespace Collett
