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

SET( SAB_ENABLE_TESTING ON CACHE BOOL "Enable Unit Testing of SAB Utils" )

#MESSAGE( STATUS "SAB_ENABLE_TESTING=${SAB_ENABLE_TESTING}" )

if( SAB_ENABLE_TESTING )
	if ( SAB_UTILS_DIR )
		SET(GOOGLETEST_ROOT_DIR ${SAB_UTILS_DIR}/googletest)
	elseif( EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/googletest )
		SET(GOOGLETEST_ROOT_DIR ${SAB_UTILS_DIR}/googletest )
	elseif( EXISTS ${CMAKE_SOURCE_DIR}/SABUtils/googletest )
		SET(GOOGLETEST_ROOT_DIR ${CMAKE_SOURCE_DIR}/SABUtils/googletest )
	else()
		MESSAGE( FATAL_ERROR "Could not find googletest root directory" )
	endif()
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
