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

cmake_minimum_required(VERSION 3.7)
if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

project(gif)
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../Modules ${CMAKE_MODULE_PATH})

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
find_package(Deploy REQUIRED)

project( gif )

AddQtIncludes()

include( ${CMAKE_CURRENT_LIST_DIR}/include.cmake )
include( ${CMAKE_CURRENT_LIST_DIR}/../Project.cmake )
add_definitions( -DBUILD_GIF_PLUGIN=1 )
add_library(gif SHARED
    ${project_SRCS} 
    ${project_H}  
    ${qtproject_SRCS} 
    ${qtproject_QRC} 
    ${qtproject_QRC_SRCS} 
    ${qtproject_UIS_H} 
    ${qtproject_MOC_SRCS} 
    ${qtproject_CPPMOC_SRCS}
    ${qtproject_CPPMOC_H} 
    ${qtproject_H} 
    ${qtproject_UIS}
    ${qtproject_QRC_SOURCES}
    ${_CMAKE_FILES}
    ${_CMAKE_MODULE_FILES}
)

target_link_libraries( gif
    Qt5::Widgets
    Qt5::Core
    SABUtils
)

set_target_properties( gif PROPERTIES 
                                RUNTIME_OUTPUT_DIRECTORY "$<CONFIG>/imageformats"
                                ARCHIVE_OUTPUT_DIRECTORY "$<CONFIG>/imageformats"
                                LIBRARY_OUTPUT_DIRECTORY "$<CONFIG>/imageformats"
                                DEBUG_POSTFIX d
                                FOLDER SharedLibs 
)

INSTALL( FILES $<TARGET_FILE_DIR:gif>/$<TARGET_FILE_BASE_NAME:gif>.dll DESTINATION imageformats )
INSTALL( FILES $<TARGET_FILE_DIR:gif>/$<TARGET_FILE_BASE_NAME:gif>.pdb DESTINATION imageformats CONFIGURATIONS Debug RelWithDebInfo )

