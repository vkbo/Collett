#
# Collett – CMake Build File
# ==========================
#
# This file is a part of Collett
# Copyright 2020–2021, Veronica Berglyd Olsen
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#
cmake_minimum_required(VERSION 3.14)

# Version
file(READ "src/collett.h" COLLET_H)
string(REGEX MATCH "#define[ ]+COL_VERSION_STR[ ]+\"([0-9a-z\.\-]+)\"" _ ${COLLET_H})
set(Collett_RELEASE ${CMAKE_MATCH_1})
string(REGEX MATCH "([0-9\.]*)" _ ${Collett_RELEASE})
set(Collett_VERSION ${CMAKE_MATCH_1})

message(STATUS "Collett Version: ${Collett_VERSION}")
message(STATUS "Collett Release: ${Collett_RELEASE}")

project(Collett LANGUAGES CXX VERSION ${Collett_VERSION})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -O2 ${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG   "-Wall -g -O0 ${CMAKE_CXX_FLAGS_DEBUG}")

# Qt6 Library
# ===========

# Note: Qt6 include folder must be available. The following assumes lib/qt6 is
# either a local copy or a symlink to it.
set(CMAKE_PREFIX_PATH "lib/qt6")

set(QT_DEFAULT_MAJOR_VERSION 6)
find_package(Qt6 COMPONENTS Core Widgets LinguistTools Svg REQUIRED)
if(Qt6Core_FOUND)
    message(STATUS "Found Qt6Core Version: ${Qt6Core_VERSION}")
endif()
if(Qt6Widgets_FOUND)
    message(STATUS "Found Qt6Widgets Version: ${Qt6Widgets_VERSION}")
endif()

# i18n
# ====

set(TS_FILES
    i18n/collett_en_US.ts
    i18n/collett_nb_NO.ts
)
qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

# Sources
# =======

# Source Folders
include_directories(
    src/core
    src/editor
    src/gui
    src/project
    src
)

# Source Files
list(APPEND SRC_FILES
    src/core/data
    src/core/icons
    src/core/storage
    src/core/svgiconengine
    src/editor/doceditor
    src/editor/edittoolbar
    src/editor/textedit
    src/gui/maintoolbar
    src/gui/statusbar
    src/gui/storytree
    src/gui/storytreedelegate
    src/gui/treetoolbar
    src/project/document
    src/project/project
    src/project/storyitem
    src/project/storymodel
    src/guimain
    src/main
    src/settings
)

# Resource Files
set(QRC_FILES
)

# Targets
# =======

qt_add_executable(Collett ${SRC_FILES} ${QRC_FILES} ${TS_FILES})
set_target_properties(Collett PROPERTIES OUTPUT_NAME "collett")
target_link_libraries(Collett PRIVATE Qt::Widgets Qt::Svg)
target_compile_definitions(Collett PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")
