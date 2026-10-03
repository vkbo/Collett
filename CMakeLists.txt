# Collett - CMake Build File
# ==========================
#
# This file is a part of Collett
# Copyright (C) 2026 Veronica Berglyd Olsen
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
cmake_minimum_required(VERSION 3.21)

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

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wall)
set(CMAKE_CXX_FLAGS_RELEASE "-O2 ${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG   "-g -O0 ${CMAKE_CXX_FLAGS_DEBUG}")

# Qt6 Library
# ===========

find_package(Qt6 6.9 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)
message(STATUS "Found Qt6 Version: ${Qt6_VERSION}")

qt_standard_project_setup(REQUIRES 6.8)

# Targets
# =======

qt_add_executable(Collett src/main.cpp)
target_include_directories(Collett PRIVATE
    src
    src/core
    src/editor
    src/project
    src/static
)
target_link_libraries(Collett PRIVATE Qt::Gui Qt::Qml Qt::Quick Qt::QuickControls2)
target_compile_definitions(Collett PRIVATE "$<$<CONFIG:Debug>:DEBUG>")

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(Collett
    URI Collett
    SOURCES
        src/collett.h
        src/core/counting.cpp
        src/core/counting.h
        src/core/icons.cpp
        src/core/icons.h
        src/core/storage.cpp
        src/core/storage.h
        src/core/tools.cpp
        src/core/tools.h
        src/editor/documentbinder.cpp
        src/editor/documentbinder.h
        src/project/document.cpp
        src/project/document.h
        src/project/group.cpp
        src/project/group.h
        src/project/node.cpp
        src/project/node.h
        src/project/project.cpp
        src/project/project.h
        src/project/projectdata.cpp
        src/project/projectdata.h
        src/project/projectmodel.cpp
        src/project/projectmodel.h
        src/project/tree.cpp
        src/project/tree.h
        src/static/settings.cpp
        src/static/settings.h
    QML_FILES
        qml/Main.qml
        qml/NewProject.qml
        qml/ProjectCard.qml
        qml/SceneEditor.qml
        qml/Theme.qml
    RESOURCES
        assets/icons/lucide.icons
)

set_target_properties(Collett PROPERTIES OUTPUT_NAME "collett")
