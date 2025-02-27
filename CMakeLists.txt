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

if(WIN32)
    include( "${CMAKE_CURRENT_LIST_DIR}/CMakeNuGetTools/cmake/NuGetTools.cmake" )
    UNSET( NUGET_COMMAND CACHE )
    find_program( 
        NUGET_COMMAND 
        nuget.exe 
        PATHS ${HOME_BIN_DIR}
        REQUIRED )
endif()

SET( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Modules ${CMAKE_MODULE_PATH})

if( SABUTILS_DLL )
#    message( STATUS "SAB Utils exists" )
    SET( SABUTILS_DLL ON CACHE BOOL "Build SAB Utils as a Shared Library" PARENT_SCOPE )
else()
#    message( STATUS "SAB Utils does not exist" )
    SET( SABUTILS_DLL ON CACHE BOOL "Build SAB Utils as a Shared Library" )
endif()

SET( MKVUTILS OFF CACHE BOOL "Support for MKV Utilities" PARENT_SCOPE )
SET( BIFSUPPORT OFF CACHE BOOL "Support for BIF Utilities" PARENT_SCOPE )
SET( GIFSUPPORT OFF CACHE BOOL "Support for GIF Utilities" PARENT_SCOPE )
SET( SAB_ENABLE_TESTING OFF CACHE BOOL "Enable Testing" PARENT_SCOPE )
   
message( STATUS "Building for DLL=${SABUTILS_DLL}" )
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

find_package(Qt5 COMPONENTS Concurrent Core Widgets Xml Network REQUIRED)
IF(WIN32)
    find_package(Qt5 COMPONENTS AxContainer REQUIRED)
ENDIF()

if ( MKVUTILS )
    find_package(Qt5 COMPONENTS Multimedia REQUIRED)
endif()

find_package(Qt5 COMPONENTS XmlPatterns QUIET)
find_package(Deploy REQUIRED)
find_package(AddUnitTest REQUIRED)
find_package(Qt5SrcMoc REQUIRED)


set(SAB_MOC_OPTIONS "-DNON_VIS_BUILD=1")

if ( MKVUTILS )
    set( MEDIAINFOLIB mediainfo )
endif()

find_package(IncludeProjectSettings REQUIRED)
include( ${CMAKE_CURRENT_LIST_DIR}/include.cmake )
project( ${_PROJECT_NAME} ) 
IncludeProjectSettings(QT ${USE_QT})

if ( MKVUTILS )
    include_directories(MediaInfo/MediaInfoLib/Source)
    include_directories(MediaInfo/ZenLib/Source)
endif()

if( WIN32 )
    nuget_initialize()

    SET( packageID Microsoft.VisualStudio.Setup.Configuration.Native )
    nuget_add_dependencies(
        PACKAGE ${packageID} VERSION 3.4.2244 CMAKE_PREFIX_PATHS .
    )
    include_directories( ${NUGET_LAST_DEPENDENCY_CMAKE_PREFIX_PATHS_${packageID}}/lib/native/include )
    set( project_pub_DEPS
        ${project_pub_DEPS}
        ${NUGET_LAST_DEPENDENCY_CMAKE_PREFIX_PATHS_${packageID}}/lib/native/v141/x64/Microsoft.VisualStudio.Setup.Configuration.Native.lib
    )
endif()

if ( SABUTILS_DLL )
    SET( LIBTYPE SHARED )
else()
    SET( LIBTYPE STATIC )
endif()

add_library(${PROJECT_NAME} ${LIBTYPE}
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
    set_target_properties( ${PROJECT_NAME} PROPERTIES 
                                FOLDER ${FOLDER_NAME} 
                                DEBUG_POSTFIX d
   ) 
    if ( WIN32 )
        INSTALL( FILES $<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_BASE_NAME:${PROJECT_NAME}>.dll DESTINATION . )
        INSTALL( FILES $<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_BASE_NAME:${PROJECT_NAME}>.pdb DESTINATION . CONFIGURATIONS Debug RelWithDebInfo )
    else()
        INSTALL( FILES $<TARGET_FILE_DIR:${PROJECT_NAME}>/lib$<TARGET_FILE_BASE_NAME:${PROJECT_NAME}>.so DESTINATION . )
    endif()
else()
    set_target_properties( ${PROJECT_NAME} PROPERTIES FOLDER ${FOLDER_NAME} )
endif()

if( SAB_ENABLE_TESTING )
    add_subdirectory(UnitTests)
endif()

if ( SABUTILS_DLL )
    DeployQt( ${PROJECT_NAME} . )
    DeploySystem( ${PROJECT_NAME} . )
endif()
