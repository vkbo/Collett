# Collett - Unit Tests
# ====================
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

# Register a Qt Test executable and its CTest entry. Tests run headless via the
# "offscreen" QPA platform so no display server is required. The tests folder
# mirrors the repository root, so the first argument is the folder of the code
# under test, like src/core or qml. Folders that do not mirror the root are
# prefixed with "_". Tests of the QML also link the QML module's plugin.
function(collett_add_test subdir name)
    qt_add_executable(${name} ${subdir}/${name}.cpp)
    target_link_libraries(${name} PRIVATE CollettCore Qt::Test)
    if(subdir STREQUAL "qml")
        target_link_libraries(${name} PRIVATE CollettCoreplugin)
    endif()
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endfunction()

collett_add_test(src/core tst_counting)
collett_add_test(src/core tst_tools)
collett_add_test(src/project tst_document)
collett_add_test(src/project tst_projectdata)
collett_add_test(src/project tst_projectmodel)
collett_add_test(src/static tst_settings)
collett_add_test(qml tst_main)
collett_add_test(qml tst_sceneeditor)
