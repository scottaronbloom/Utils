#
# (c) Copyright 2004 - 2015 Blue Pearl Software Inc.
# All rights reserved.
#
# This source code belongs to Blue Pearl Software Inc.
# It is considered trade secret and confidential, and is not to be used
# by parties who have not received written authorization
# from Blue Pearl Software Inc.
#
# Only authorized users are allowed to use, copy and modify
# this software provided that the above copyright notice
# remains in all copies of this software.
#
#
# $Author: scott $ - $Revision: 55494 $ - $Date: 2019-10-16 15:17:25 -0700 (Wed, 16 Oct 2019) $
# $HeadURL: http://bpsvn/svn/trunk/Modules/FindBPSUnitTest.cmake $
#
#
#
FUNCTION(BPS_UNIT_TEST_RESOURCE name)
    STRING(REPLACE "${CMAKE_SOURCE_DIR}" "" LCL_DIR ${CMAKE_CURRENT_LIST_DIR})
    STRING(REPLACE "CoreApp" "CLI" LCL_DIR ${LCL_DIR})
    STRING(REPLACE "/UnitTests" "" LCL_DIR ${LCL_DIR})
    SET(FOLDER_NAME ${LCL_DIR})
    STRING(REGEX REPLACE "/(.*)/(.*)" "BPSTests/UnitTests/\\1/\\2" FOLDER_NAME ${FOLDER_NAME})

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

FUNCTION(BPS_UNIT_TEST name file libs)
    STRING(REPLACE "${CMAKE_SOURCE_DIR}" "" LCL_DIR ${CMAKE_CURRENT_LIST_DIR})
    STRING(REPLACE "CoreApp" "CLI" LCL_DIR ${LCL_DIR})
    STRING(REPLACE "/UnitTests" "" LCL_DIR ${LCL_DIR})
    SET(FOLDER_NAME ${LCL_DIR})
    #MESSAGE( "INIT FOLDER_NAME=${FOLDER_NAME}" )
    STRING(REGEX REPLACE "/(.*)/(.*)" "\\1" TOP_FOLDER ${FOLDER_NAME})
    STRING(REGEX REPLACE "\/" "" TOP_FOLDER ${TOP_FOLDER})

    STRING(REGEX REPLACE "/(.*)/(.*)" "BPSTests/UnitTests/\\1/\\2" FOLDER_NAME ${FOLDER_NAME})

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
    #MESSAGE( "FOLDER_NAME=${FOLDER_NAME}" )
    set_target_properties( ${TEST_NAME} PROPERTIES FOLDER ${FOLDER_NAME})
    SET (NEWPATH "${CMAKE_BINARY_DIR}/tcl/src/Debug;${CMAKE_BINARY_DIR}/tcl/src/RelWithDebInfo;${QTDIR}/bin;${OPENSSL_ROOT_DIR};$ENV{PATH}" )
    STRING(REPLACE ";" "\\;" NEWPATH "${NEWPATH}")
    SET_TESTS_PROPERTIES( ${TEST_NAME} PROPERTIES ENVIRONMENT "PATH=${NEWPATH}" )

    #MESSAGE( "TOP_FOLDER=${TOP_FOLDER}" )
    #MESSAGE( "          =BPSTests_${TOP_FOLDER}" )
    #MESSAGE( "          =${TEST_NAME}" )
    ADD_DEPENDENCIES(BPSTests_${TOP_FOLDER} ${TEST_NAME} )
    set_target_properties( BPSTests_${TOP_FOLDER} PROPERTIES FOLDER CMakePredefinedTargets/UnitTests )

    CreateUserProj( Test_${PROJECT_NAME} )
ENDFUNCTION()

