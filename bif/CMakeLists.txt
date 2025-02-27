# The MIT License (MIT)
#
# Copyright (c) 2020-2021 Scott Aron Bloom
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.30)

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../Modules ${CMAKE_MODULE_PATH})

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
find_package(Deploy REQUIRED)

find_package(IncludeProjectSettings REQUIRED)
include( ${CMAKE_CURRENT_LIST_DIR}/include.cmake )
project( ${_PROJECT_NAME} ) 
IncludeProjectSettings(QT ${USE_QT})

add_definitions( -DBUILD_BIF_PLUGIN=1 )
add_definitions( -DSABUTILS_INTERNAL )

add_library(${PROJECT_NAME} SHARED
    ${_PROJECT_DEPENDENCIES} 
)

target_link_libraries( ${PROJECT_NAME}
    PUBLIC
        ${project_pub_DEPS}
    PRIVATE 
        ${project_pri_DEPS}
)

set_target_properties( ${PROJECT_NAME} PROPERTIES 
                                RUNTIME_OUTPUT_DIRECTORY "$<CONFIG>/imageformats"
                                ARCHIVE_OUTPUT_DIRECTORY "$<CONFIG>/imageformats"
                                LIBRARY_OUTPUT_DIRECTORY "$<CONFIG>/imageformats"
                                DEBUG_POSTFIX d
                                FOLDER ${FOLDER_NAME} 
)

INSTALL( FILES $<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_BASE_NAME:${PROJECT_NAME}>.dll DESTINATION imageformats )
INSTALL( FILES $<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_BASE_NAME:${PROJECT_NAME}>.pdb DESTINATION imageformats CONFIGURATIONS Debug RelWithDebInfo )

