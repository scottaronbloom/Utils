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

FUNCTION(SAB_UNIT_TEST_RESOURCE name)
    STRING(REPLACE "${CMAKE_SOURCE_DIR}" "" LCL_DIR ${CMAKE_CURRENT_LIST_DIR})
    STRING(REPLACE "/UnitTests" "" LCL_DIR ${LCL_DIR})
    SET(FOLDER_NAME "UnitTests/${LCL_DIR}")
    STRING(REGEX REPLACE "/(.*)/(.*)" "SABTests/UnitTests/\\1/\\2" FOLDER_NAME ${FOLDER_NAME})

    #MESSAGE( "FOLDER_NAME=${FOLDER_NAME}" )

    SET(RESOURCE_LIB_NAME ${LCL_DIR}_${name} ) 

    STRING(SUBSTRING ${RESOURCE_LIB_NAME} 1 -1 RESOURCE_LIB_NAME)
    STRING(REGEX REPLACE "(.*)/(.*)" "\\2" RESOURCE_LIB_NAME ${RESOURCE_LIB_NAME})
    #MESSAGE( "RESOURCE_LIB_NAME=${RESOURCE_LIB_NAME}" )

    project( ${RESOURCE_LIB_NAME} )

    if(POLICY CMP0020)
        cmake_policy(SET CMP0020 NEW)
    endif()

    QT5_ADD_RESOURCES( qt_project_QRC_SRCS ${ARGN} )
    add_library(${RESOURCE_LIB_NAME} STATIC ${qt_project_QRC_SRCS})
    set_target_properties( ${RESOURCE_LIB_NAME} PROPERTIES FOLDER ${FOLDER_NAME})

    set( ${RESOURCE_LIB_NAME} ${RESOURCE_LIB_NAME} PARENT_SCOPE )
ENDFUNCTION()

FUNCTION(SAB_UNIT_TEST name file libs)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED true)

    STRING(REPLACE "${CMAKE_SOURCE_DIR}" "" LCL_DIR ${CMAKE_CURRENT_LIST_DIR})
    STRING(REPLACE "CoreApp" "CLI" LCL_DIR ${LCL_DIR})
    STRING(REPLACE "/UnitTests" "" LCL_DIR ${LCL_DIR})
    SET(FOLDER_NAME "${LCL_DIR}")
    #MESSAGE( "INIT FOLDER_NAME=${FOLDER_NAME}" )
    STRING(REGEX REPLACE "/(.*)/(.*)" "\\1" TOP_FOLDER ${FOLDER_NAME})
    STRING(REGEX REPLACE "\/" "" TOP_FOLDER ${TOP_FOLDER})

    STRING(REGEX REPLACE "/(.*)/(.*)" "SABTests/UnitTests/\\1/\\2" FOLDER_NAME ${FOLDER_NAME})

    SET( TEST_NAME ${LCL_DIR}_${name} ) 
    STRING(SUBSTRING ${TEST_NAME} 1 -1 TEST_NAME)
    STRING(REGEX REPLACE "(.*)/(.*)" "\\2" TEST_NAME ${TEST_NAME})
    project( ${TEST_NAME} )
    # message( "Adding unit test ${TEST_NAME}" )
    if(POLICY CMP0020)
        cmake_policy(SET CMP0020 NEW)
    endif()
    #MESSAGE( "ARGN=${ARGN}" )
    add_executable(${TEST_NAME} ${file} ${ARGN})
    target_link_libraries(${TEST_NAME} ${libs})
    add_test(${TEST_NAME} ${TEST_NAME})
    #MESSAGE( "TEST_NAME=${TEST_NAME}" )
    SET( FOLDER_NAME "UnitTests/${FOLDER_NAME}" )
    MESSAGE( "FOLDER_NAME=${FOLDER_NAME}" )
    set_target_properties( ${TEST_NAME} PROPERTIES FOLDER ${FOLDER_NAME})
    SET (NEWPATH "${CMAKE_BINARY_DIR}/tcl/src/Debug;${CMAKE_BINARY_DIR}/tcl/src/RelWithDebInfo;${QTDIR}/bin;${OPENSSL_ROOT_DIR};$ENV{PATH}" )
    STRING(REPLACE ";" "\\;" NEWPATH "${NEWPATH}")
    SET_TESTS_PROPERTIES( ${TEST_NAME} PROPERTIES ENVIRONMENT "PATH=${NEWPATH}" )

    #MESSAGE( "TOP_FOLDER=${TOP_FOLDER}" )
    #MESSAGE( "          =SABTests_${TOP_FOLDER}" )
    #MESSAGE( "          =${TEST_NAME}" )
    ADD_DEPENDENCIES(SABTests_${TOP_FOLDER} ${TEST_NAME} )
    set_target_properties( SABTests_${TOP_FOLDER} PROPERTIES FOLDER CMakePredefinedTargets/UnitTests )

    #CreateUserProj( Test_${PROJECT_NAME} )
ENDFUNCTION()

