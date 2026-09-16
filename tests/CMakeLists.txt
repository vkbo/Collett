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
# mirrors the src folder, so the first argument is the src subfolder of the
# code under test. Folders that do not mirror src are prefixed with "_".
function(collett_add_test subdir name)
    qt_add_executable(${name} ${subdir}/${name}.cpp)
    target_link_libraries(${name} PRIVATE CollettCore Qt::Test)
    target_compile_definitions(${name} PRIVATE
        TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/_data"
        ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
    )
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endfunction()

collett_add_test(core tst_counting)
collett_add_test(editor tst_highlighter)
collett_add_test(editor tst_textcheck)
collett_add_test(editor tst_texteditor)
collett_add_test(layouts tst_scrollableform)
collett_add_test(project tst_document)
collett_add_test(project tst_projectdata)
collett_add_test(project tst_projectmodel)
collett_add_test(spellcheck tst_spellcheck)
collett_add_test(static tst_settings)
collett_add_test(static tst_theme)
collett_add_test(widgets tst_mcombobox)
collett_add_test(widgets tst_mpagedsidebar)
collett_add_test(widgets tst_mswitch)
