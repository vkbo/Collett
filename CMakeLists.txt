cmake_minimum_required(VERSION 3.5)

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
find_package(Qt6 COMPONENTS Core Widgets Xml LinguistTools REQUIRED)
if(Qt6Widgets_FOUND)
    message(STATUS "Found Qt6Widgets Libraries: ${Qt6Widgets_LIBRARIES}")
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
    src
    src/document
    src/editor
    src/gui
    src/project
    src/tools
)

# Source Files
list(APPEND Collett_SOURCE
    document/document
    document/documentblock
    editor/doceditor
    gui/mainmenu
    gui/maintoolbar
    gui/noveltree
    gui/noveltreedelegate
    gui/statusbar
    project/project
    project/storyitem
    project/storymodel
    project/storytree
    tools/textutils
    data
    guimain
    main
    settings
)

# Resource Files
set(QRC_FILES
    assets/icons.qrc
)

foreach(entry IN LISTS Collett_SOURCE)
    list(APPEND EXEC_SRC src/${entry}.cpp)
endforeach()

# Targets
# =======

qt_add_executable(Collett ${EXEC_SRC} ${QRC_FILES} ${TS_FILES})
target_link_libraries(Collett PRIVATE Qt::Widgets Qt::Xml)
set_target_properties(Collett PROPERTIES OUTPUT_NAME "collett")
