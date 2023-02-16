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

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
find_package(Threads REQUIRED)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

source_group("Header Files" FILES ${project_H} )
source_group("Source Files" FILES ${project_SRCS} )

SET( _CMAKE_FILES "CMakeLists.txt;include.cmake" )
source_group("CMake Files" FILES ${_CMAKE_FILES} )
FILE(GLOB _CMAKE_MODULE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Modules/*")
source_group("CMake Files\\Modules" FILES ${_CMAKE_MODULE_FILES} )

if( EXISTS "${SAB_UTILS_DIR}/CompilerSettings.cmake" )
    include( ${SAB_UTILS_DIR}/CompilerSettings.cmake )
endif()

if( EXISTS "${CMAKE_SOURCE_DIR}/SABUTILS/CompilerSettings.cmake" )
    include( ${CMAKE_SOURCE_DIR}/SABUtils/CompilerSettings.cmake )
endif()

SET( _PROJECT_DEPENDENCIES
    ${project_SRCS} 
    ${project_H}  
    ${_CMAKE_FILES}
    ${_CMAKE_MODULE_FILES}
)

SET( project_pub_DEPS
    # insert and "global default" public depends here
     ${project_pub_DEPS}
     )

SET( project_pri_DEPS
    # insert and "global default" private depends here
    ${project_pri_DEPS}
)
