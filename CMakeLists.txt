# Collett – CMake Build File
# ==========================
#
# This file is a part of Collett
# Copyright (C) 2025 Veronica Berglyd Olsen
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
cmake_minimum_required(VERSION 3.19)

# Version
file(READ "src/collett.h" COLLET_H)
string(REGEX MATCH "#define[ ]+COL_VERSION_STR[ ]+\"([0-9a-z\.\-]+)\"" _ ${COLLET_H})
set(Collett_RELEASE ${CMAKE_MATCH_1})
string(REGEX MATCH "([0-9\.]*)" _ ${Collett_RELEASE})
set(Collett_VERSION ${CMAKE_MATCH_1})
set (CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build.")

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
# set(CMAKE_PREFIX_PATH "lib/qt6/6.5.0/gcc_64/lib/cmake/")

cmake_policy(SET CMP0115 OLD)
set(QT_DEFAULT_MAJOR_VERSION 6)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Svg LinguistTools)
if(Qt6Core_FOUND)
    message(STATUS "Found Qt6Core Version: ${Qt6Core_VERSION}")
endif()
if(Qt6Widgets_FOUND)
    message(STATUS "Found Qt6Widgets Version: ${Qt6Widgets_VERSION}")
endif()

# i18n
# ====

qt_standard_project_setup(
    I18N_SOURCE_LANGUAGE en_GB
    I18N_TRANSLATED_LANGUAGES en_US nb_NO de
)

# set(TS_FILES
#     i18n/collett_en_US.ts
#     i18n/collett_nb_NO.ts
# )
# qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

# Sources
# =======

# Source Folders
include_directories(
    src/core
    src/project
    src
)

# Source Files
list(APPEND SRC_FILES
    src/core/data
    src/core/storage
    src/project/project
    src/project/projectdata
    src/guimain
    src/main
)

# Targets
# =======

qt_add_executable(Collett ${SRC_FILES})
qt_add_translations(Collett TS_FILE_DIR i18n)
qt_add_resources(Collett "assets" PREFIX "/styles" BASE "assets/" FILES
    assets/styles.qss
)

set_target_properties(Collett PROPERTIES OUTPUT_NAME "collett")
target_link_libraries(Collett PRIVATE Qt::Widgets Qt::Svg)
target_compile_definitions(Collett PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")
