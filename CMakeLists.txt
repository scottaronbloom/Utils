# The MIT License (MIT)
#
# Copyright (c) 2020 Scott Aron Bloom
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

cmake_minimum_required(VERSION 3.1)
if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

project(SABUtils)
if ( NOT SAB_UTILS_DIR )
    SET( SAB_UTILS_DIR ${CMAKE_CURRENT_LIST_DIR} )
endif()

if( SAB_ENABLE_TESTING )
    SET(GOOGLETEST_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/googletest)
    MESSAGE( STATUS "Adding support for testing - ${GOOGLETEST_ROOT_DIR}" )
    enable_testing()
    add_subdirectory( ${GOOGLETEST_ROOT_DIR} )
    SET(GOOGLETEST_SOURCE_DIR ${GOOGLETEST_ROOT_DIR}/googletest)
    SET(GMOCK_SOURCE_DIR ${GOOGLETEST_ROOT_DIR}/googlemock)
    option( gtest_force_shared_crt "Use shared ( DLL ) run-time lib even when Google Test is built as static lib." ON )
    set_target_properties( gmock PROPERTIES FOLDER 3rdParty/google )
    set_target_properties( gmock_main PROPERTIES FOLDER 3rdParty/google )
    set_target_properties( gtest PROPERTIES FOLDER 3rdParty/google )
    set_target_properties( gtest_main PROPERTIES FOLDER 3rdParty/google )
endif()

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Modules ${CMAKE_MODULE_PATH})

OPTION(SAB_DEBUG_TRACE "Enable Debug Tracing to std::cout" OFF)

find_package(Qt5 COMPONENTS Core Widgets Xml XmlPatterns REQUIRED)
find_package(Deploy REQUIRED)
find_package(AddUnitTest REQUIRED)
find_package(Qt5SrcMoc REQUIRED)

if ( SAB_ENABLE_TESTING )
endif()

project( SABUtils )

AddQtIncludes()

include( ${CMAKE_CURRENT_LIST_DIR}/include.cmake )
include( ${CMAKE_CURRENT_LIST_DIR}/Project.cmake )

add_library(SABUtils STATIC
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
set_target_properties( SABUtils PROPERTIES FOLDER Libs )
add_subdirectory(UnitTests)
add_subdirectory(QtUnitTests)

