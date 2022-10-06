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

cmake_minimum_required(VERSION 3.21)
if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

project(SABUtils)

SET( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Modules ${CMAKE_MODULE_PATH} )

SET( SABUTILS_DLL ON CACHE BOOL "Build SAB Utils as a Shared Library" )
SET( MKVUTILS OFF CACHE BOOL "Support for MKV Utilities" )
SET( BIFSUPPORT OFF CACHE BOOL "Support for BIF Utilities" )
SET( GIFSUPPORT OFF CACHE BOOL "Support for GIF Utilities" )
SET( SAB_ENABLE_TESTING OFF CACHE BOOL "Enable Testing" )
   
message( STATUS "Building DLL=${SABUTILS_DLL}" )
message( STATUS "Building MKVUTILS=${MKVUTILS}" )
message( STATUS "Building BIFSUPPORT=${BIFSUPPORT}" )
message( STATUS "Building GIFSUPPORT=${GIFSUPPORT}" )
message( STATUS "Building SAB_ENABLE_TESTING=${SAB_ENABLE_TESTING}" )

if ( NOT SAB_UTILS_DIR )
    SET( SAB_UTILS_DIR ${CMAKE_CURRENT_LIST_DIR} )
endif()

add_definitions( -DSABUTILS_INTERNAL )

include( Modules/EnableGoogleTesting.cmake )

if ( MKVUTILS )
    SET(NO_USE_MEDIAINFO_INSTALL ON)
    set(BUILD_SHARED_LIBS ON)
	option(LARGE_FILES "Enable large files support" OFF)
    add_subdirectory( MediaInfo/ProjectFiles/MediaInfoLib )
    add_subdirectory( MKVReader )
endif()


OPTION(SAB_DEBUG_TRACE "Enable Debug Tracing to std::cout" OFF)

find_package(Qt5 COMPONENTS Core Widgets Xml Concurrent Network REQUIRED)
find_package(Qt5 COMPONENTS XmlPatterns QUIET)
find_package(Deploy REQUIRED)
find_package(AddUnitTest REQUIRED)
find_package(Qt5SrcMoc REQUIRED)

project( SABUtils )

set(SAB_MOC_OPTIONS "-DNON_VIS_BUILD=1")

if ( MKVUTILS )
    set( MEDIAINFOLIB mediainfo )
endif()

include( ${CMAKE_CURRENT_LIST_DIR}/include.cmake )
include( ${CMAKE_CURRENT_LIST_DIR}/Project.cmake )

if ( MKVUTILS )
    include_directories(MediaInfo/MediaInfoLib/Source)
    include_directories(MediaInfo/ZenLib/Source)
endif()

if ( SABUTILS_DLL )
    SET( LIBTYPE SHARED )
else()
    SET( LIBTYPE STATIC )
endif()

add_library(SABUtils ${LIBTYPE}
    ${_PROJECT_DEPENDENCIES}
)

target_link_libraries( ${PROJECT_NAME}
    PUBLIC
        ${project_pub_DEPS}
    PRIVATE 
        ${project_pri_DEPS}
)

add_definitions( -DNON_VIS_BUILD=1 )
if ( BIFSUPPORT )
    add_definitions( -DBIF_SCROLLBAR_SUPPORT=1 )
endif()

if ( SABUTILS_DLL )
    set_target_properties( SABUtils PROPERTIES 
                                FOLDER SharedLibs 
                                DEBUG_POSTFIX d
   ) 
    if ( WIN32 )
        INSTALL( FILES $<TARGET_FILE_DIR:SABUtils>/$<TARGET_FILE_BASE_NAME:SABUtils>.dll DESTINATION . )
        INSTALL( FILES $<TARGET_FILE_DIR:SABUtils>/$<TARGET_FILE_BASE_NAME:SABUtils>.pdb DESTINATION . CONFIGURATIONS Debug RelWithDebInfo )
    else()
        INSTALL( FILES $<TARGET_FILE_DIR:SABUtils>/lib$<TARGET_FILE_BASE_NAME:SABUtils>.so DESTINATION . )
    endif()
else()
    set_target_properties( SABUtils PROPERTIES FOLDER Libs )
endif()

if( SAB_ENABLE_TESTING )
    add_subdirectory(UnitTests)
endif()

if ( SABUTILS_DLL )
    DeployQt( SABUtils . )
    DeploySystem( SABUtils . )
endif()
