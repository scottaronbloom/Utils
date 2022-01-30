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

cmake_minimum_required(VERSION 3.1)
if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

project(SABUtils)

SET( SABUTILS_DLL ON CACHE BOOL "Build SAB Utils as a Shared Library" )
   
if ( NOT SAB_UTILS_DIR )
    SET( SAB_UTILS_DIR ${CMAKE_CURRENT_LIST_DIR} )
endif()

add_definitions( -DSABUTILS_INTERNAL )

MESSAGE( STATUS "SAB_ENABLE_TESTING=${SAB_ENABLE_TESTING}" )

if( SAB_ENABLE_TESTING )
    SET(GOOGLETEST_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/googletest)
    MESSAGE( STATUS "Adding support for testing - ${GOOGLETEST_ROOT_DIR}" )
    SET( gtest_force_shared_crt ON CACHE BOOL "Always use shared crt" )
    UNSET( INSTALL_GTEST CACHE )
    SET( INSTALL_GTEST OFF CACHE BOOL "Never install gtest" )
    if ( NOT gtest_force_shared_crt )
        message( FATAL_ERROR "Please run with -Dgtest_force_shared_crt=ON" )
    endif()
    if ( INSTALL_GTEST )
        message( FATAL_ERROR "Please run with -DINSTALL_GTEST=OFF" )
    endif()
    
    enable_testing()
    
    add_subdirectory( ${GOOGLETEST_ROOT_DIR} )
    SET(GOOGLETEST_SOURCE_DIR ${GOOGLETEST_ROOT_DIR}/googletest)
    SET(GMOCK_SOURCE_DIR ${GOOGLETEST_ROOT_DIR}/googlemock)
    set_target_properties( gmock PROPERTIES FOLDER GoogleTest )
    set_target_properties( gmock_main PROPERTIES FOLDER GoogleTest )
    set_target_properties( gtest PROPERTIES FOLDER GoogleTest )
    set_target_properties( gtest_main PROPERTIES FOLDER GoogleTest )
endif()

SET(NO_USE_MEDIAINFO_INSTALL ON)
set(BUILD_SHARED_LIBS ON)
add_subdirectory( MediaInfo/ProjectFiles/MediaInfoLib )

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Modules ${CMAKE_MODULE_PATH})

OPTION(SAB_DEBUG_TRACE "Enable Debug Tracing to std::cout" OFF)

find_package(Qt5 COMPONENTS Core Widgets Xml Concurrent REQUIRED)
find_package(Qt5 COMPONENTS XmlPatterns QUIET)
find_package(Deploy REQUIRED)
find_package(AddUnitTest REQUIRED)
find_package(Qt5SrcMoc REQUIRED)

project( SABUtils )

AddQtIncludes()
set(SAB_MOC_OPTIONS "-DNON_VIS_BUILD=1")

include( ${CMAKE_CURRENT_LIST_DIR}/include.cmake )
include( ${CMAKE_CURRENT_LIST_DIR}/Project.cmake )

include_directories(MediaInfo/MediaInfoLib/Source)
include_directories(MediaInfo/ZenLib/Source)

if ( SABUTILS_DLL )
    SET( LIBTYPE SHARED )
else()
    SET( LIBTYPE STATIC )
endif()
add_library(SABUtils ${LIBTYPE}
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

if ( SABUTILS_DLL )
target_link_libraries( SABUtils
                 Qt5::Widgets
                 Qt5::Core
                 Qt5::Network
                 mediainfo
          )
endif()

add_definitions( -DNON_VIS_BUILD=1 )
add_definitions( -DBIF_SCROLLBAR_SUPPORT=1 )
if ( SABUTILS_DLL )
set_target_properties( SABUtils PROPERTIES FOLDER SharedLibs )
INSTALL( FILES $<TARGET_FILE_DIR:SABUtils>/$<TARGET_FILE_BASE_NAME:SABUtils>.dll DESTINATION . )
INSTALL( FILES $<TARGET_FILE_DIR:SABUtils>/$<TARGET_FILE_BASE_NAME:SABUtils>.pdb DESTINATION . CONFIGURATIONS Debug RelWithDebInfo )
else()
set_target_properties( SABUtils PROPERTIES FOLDER Libs )
endif()

if( SAB_ENABLE_TESTING )
	add_subdirectory(UnitTests)
endif()
