# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

declare_module(notateai)

set(MODULE_QRC notateai.qrc)
set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/notateaimodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/notateaimodule.h
    ${CMAKE_CURRENT_LIST_DIR}/notateaitypes.h
    ${CMAKE_CURRENT_LIST_DIR}/inotateaiconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notateaiconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notateaiconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/geminiservice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/geminiservice.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/commandparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/commandparser.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/commandexecutor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/commandexecutor.h
    ${CMAKE_CURRENT_LIST_DIR}/view/notateaipanelmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/notateaipanelmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/notateaipreferencesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/notateaipreferencesmodel.h
    )

set(MODULE_LINK
    muse::uicomponents
    muse::network
    muse::extensions
    Qt::Quick
    Qt::Concurrent
    )

setup_module()
