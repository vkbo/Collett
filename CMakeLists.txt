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

find_package(Qt6 6.9 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Test LinguistTools)
message(STATUS "Found Qt6 Version: ${Qt6_VERSION}")

# Nuspell Library
# ===============

# Spell checking engine. Reads the same .aff/.dic dictionaries as Hunspell.
find_package(Nuspell REQUIRED)
message(STATUS "Found Nuspell Version: ${Nuspell_VERSION}")

qt_standard_project_setup(REQUIRES 6.8)

# Coverage
# ========

option(COLLETT_COVERAGE "Instrument the code for gcov coverage" OFF)
if(COLLETT_COVERAGE)
    message(STATUS "Coverage instrumentation: ON")
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)

    # A "coverage" target that runs the tests and renders a report of the C++
    # code in src. Requires gcovr, and is only defined when it is found.
    find_program(GCOVR_EXECUTABLE gcovr)
    if(GCOVR_EXECUTABLE)
        add_custom_target(coverage
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
            COMMAND ${GCOVR_EXECUTABLE}
                --root ${CMAKE_SOURCE_DIR}
                --filter ${CMAKE_SOURCE_DIR}/src/
                --exclude ".*autogen.*"
                --gcov-ignore-parse-errors
                --txt ${CMAKE_BINARY_DIR}/coverage/coverage.txt
                --markdown-heading-level 4
                --markdown ${CMAKE_BINARY_DIR}/coverage/coverage.md
                --html-details ${CMAKE_BINARY_DIR}/coverage/index.html
                --cobertura-pretty --cobertura ${CMAKE_BINARY_DIR}/coverage/coverage.xml
                --lcov ${CMAKE_BINARY_DIR}/coverage/lcov.info
                ${CMAKE_BINARY_DIR}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running tests and generating coverage report"
            USES_TERMINAL
        )
    else()
        message(STATUS "gcovr not found; 'coverage' target unavailable")
    endif()
endif()

# Targets
# =======

# Core library: the C++ code and QML files, as the backing target of the QML
# module, so the tests can link it without the application's main function
qt_add_library(CollettCore STATIC)
target_include_directories(CollettCore PUBLIC
    src
    src/core
    src/editor
    src/project
    src/spellcheck
    src/static
)
target_link_libraries(CollettCore PUBLIC Qt::Gui Qt::Qml Qt::Quick Qt::QuickControls2 PRIVATE Nuspell::nuspell)
target_compile_definitions(CollettCore PUBLIC "$<$<CONFIG:Debug>:DEBUG>")

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(CollettCore
    URI Collett
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Collett
    SOURCES
        src/collett.h
        src/core/counting.cpp
        src/core/counting.h
        src/core/fonts.cpp
        src/core/fonts.h
        src/core/icons.cpp
        src/core/icons.h
        src/core/labels.cpp
        src/core/labels.h
        src/core/storage.cpp
        src/core/storage.h
        src/core/tools.cpp
        src/core/tools.h
        src/editor/documentbinder.cpp
        src/editor/documentbinder.h
        src/editor/highlighter.cpp
        src/editor/highlighter.h
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
        src/spellcheck/nuspellengine.cpp
        src/spellcheck/nuspellengine.h
        src/spellcheck/spellchecker.cpp
        src/spellcheck/spellchecker.h
        src/spellcheck/spellengine.cpp
        src/spellcheck/spellengine.h
        src/spellcheck/textcheck.cpp
        src/spellcheck/textcheck.h
        src/spellcheck/userdictionary.cpp
        src/spellcheck/userdictionary.h
        src/static/settings.cpp
        src/static/settings.h
    QML_FILES
        qml/EditorToolBar.qml
        qml/FontPage.qml
        qml/GroupEditor.qml
        qml/Main.qml
        qml/NewProject.qml
        qml/PreferencesDialog.qml
        qml/ProjectItem.qml
        qml/SettingsGroup.qml
        qml/SettingsLinkRow.qml
        qml/SettingsRow.qml
        qml/SettingsSection.qml
        qml/Theme.qml
    RESOURCES
        assets/icons/lucide.icons
)

# Executable
# The style settings for Qt Quick Controls, which Qt reads from the root of
# the resources. They are in the core library, so the tests use them too.
qt_add_resources(CollettCore "controls"
    PREFIX "/"
    BASE assets
    FILES assets/qtquickcontrols2.conf
)

qt_add_executable(Collett src/main.cpp)
target_link_libraries(Collett PRIVATE CollettCore CollettCoreplugin)
set_target_properties(Collett PROPERTIES OUTPUT_NAME "collett")

# Translations
# ============
# The translation sources are updated with "pkgutils.py qtlupdate", and built
# into the app at :/i18n. The base file is only for the translation platform.

file(GLOB COLLETT_TS_FILES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/i18n/collett_*.ts)
list(FILTER COLLETT_TS_FILES EXCLUDE REGEX "collett_base\\.ts$")
if(COLLETT_TS_FILES)
    qt_add_lrelease(
        TS_FILES ${COLLETT_TS_FILES}
        QM_FILES_OUTPUT_VARIABLE COLLETT_QM_FILES
    )
    qt_add_resources(Collett "translations"
        PREFIX "/i18n"
        BASE ${CMAKE_CURRENT_BINARY_DIR}
        FILES ${COLLETT_QM_FILES}
    )
endif()

# Tests
# =====

option(COLLETT_BUILD_TESTS "Build the Collett unit tests" ON)
if(COLLETT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
