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
FUNCTION( GetVersionInfo FILENAME VAR )
	MESSAGE( STATUS "Loading version info from ${FILENAME}" )
	
	FILE( STRINGS ${FILENAME} _VERSION_FILE )

	foreach( curr ${_VERSION_FILE} )
		if( curr MATCHES "MAJOR_VERSION = ([0-9]+)" )
			SET(${VAR}_MAJOR ${CMAKE_MATCH_1})
			SET(${VAR}_MAJOR ${CMAKE_MATCH_1} PARENT_SCOPE)
		elseif ( curr MATCHES "MINOR_VERSION = ([0-9]+)" )
			SET(${VAR}_MINOR ${CMAKE_MATCH_1})
			SET(${VAR}_MINOR ${CMAKE_MATCH_1} PARENT_SCOPE)
		elseif ( curr MATCHES "PATCH_VERSION = ([0-9]+)" )
			SET(${VAR}_PATCH ${CMAKE_MATCH_1})
			SET(${VAR}_PATCH ${CMAKE_MATCH_1} PARENT_SCOPE)
		elseif ( curr MATCHES "APP_NAME = \"(.+)\"" )
			SET(${VAR}_APPNAME ${CMAKE_MATCH_1})
			SET(${VAR}_APPNAME ${CMAKE_MATCH_1} PARENT_SCOPE)
		elseif ( curr MATCHES "VENDOR = \"(.+)\"" )
			SET(${VAR}_VENDOR ${CMAKE_MATCH_1})
			SET(${VAR}_VENDOR ${CMAKE_MATCH_1} PARENT_SCOPE)
		elseif ( curr MATCHES "HOMEPAGE = \"(.+)\"" )
			SET(${VAR}_HOMEPAGE ${CMAKE_MATCH_1})
			SET(${VAR}_HOMEPAGE ${CMAKE_MATCH_1} PARENT_SCOPE)
		elseif ( curr MATCHES "PRODUCT_HOMEPAGE = \"(.+)\"" )
			SET(${VAR}_PRODUCT_HOMEPAGE ${CMAKE_MATCH_1})
			SET(${VAR}_PRODUCT_HOMEPAGE ${CMAKE_MATCH_1} PARENT_SCOPE)
		elseif ( curr MATCHES "EMAIL = \"(.+)\"" )
			SET(${VAR}_EMAIL ${CMAKE_MATCH_1})
			SET(${VAR}_EMAIL ${CMAKE_MATCH_1} PARENT_SCOPE)
		endif()
	endforeach()
	
	add_custom_command( 
		OUTPUT
			${FILENAME}
		MAIN_DEPENDENCY
			${FILENAME}
		COMMAND
			"${CMAKE_COMMAND}" -E touch ${CMAKE_SOURCE_DIR}/CMakeLists.txt
		COMMENT
			"Re-running CMake since Version.h has been modified"
	)
	
	MESSAGE( STATUS ${${VAR}_MAJOR} )
	MESSAGE( STATUS ${${VAR}_MINOR} )
	MESSAGE( STATUS ${${VAR}_PATCH} )
	MESSAGE( STATUS ${${VAR}_APPNAME} )
	MESSAGE( STATUS ${${VAR}_VENDOR} )
	MESSAGE( STATUS ${${VAR}_HOMEPAGE} )
	MESSAGE( STATUS ${${VAR}_PRODUCT_HOMEPAGE} )
	MESSAGE( STATUS ${${VAR}_EMAIL} )
ENDFUNCTION()

