# The MIT License (MIT)
#
# Copyright (c) 2017 Nathan Osman
# Copyright (c) 2020-2021 Scott Aron Bloom - Work on linux and mac
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


## https://stackoverflow.com/questions/32183975/how-to-print-all-the-properties-of-a-target-in-cmake/56738858#56738858
## https://stackoverflow.com/a/56738858/3743145

## Get all properties that cmake supports
function(print_target_properties tgt)
	execute_process(COMMAND cmake --help-property-list OUTPUT_VARIABLE __CMAKE_PROPERTY_LIST)
	## Convert command output into a CMake list
	STRING(REGEX REPLACE ";" "\\\\;" __CMAKE_PROPERTY_LIST "${__CMAKE_PROPERTY_LIST}")
	STRING(REGEX REPLACE "\n" ";" __CMAKE_PROPERTY_LIST "${__CMAKE_PROPERTY_LIST}")

	list(REMOVE_DUPLICATES __CMAKE_PROPERTY_LIST)

    if(NOT TARGET ${tgt})
      message("There is no target named '${tgt}'")
      return()
    endif()

    foreach (prop ${__CMAKE_PROPERTY_LIST})
        string(REPLACE "<CONFIG>" "${CMAKE_BUILD_TYPE}" prop ${prop})
        get_target_property(propval ${tgt} ${prop})
        if (propval)
            message ("${tgt} ${prop} = ${propval}")
        endif()
    endforeach(prop)
endfunction(print_target_properties)

if( NOT WIN32 )
    find_package( OpenSSL REQUIRED )
endif()

function( CheckOpenSSL )
	if ( NOT DEFINED OPENSSL_DEPLOY_LIBS )
		if ( NOT DEFINED OPENSSL_FOUND ) 
			if ( DEFINED OPENSSL_ROOT_DIR )
				MESSAGE( STATUS "OPENSSL_ROOT_DIR is set, calling find_package( OpenSSL REQUIRED )" )
				find_package( OpenSSL REQUIRED )
			elseif ( WIN32 )
				MESSAGE( STATUS
					  " Neither OPENSSL_FOUND and OPENSSL_ROOT_DIR are set, checking default locations." )

				if( EXISTS "C:/OpenSSL/openssl-1.1" )  # Only valid version
					if(CMAKE_SIZEOF_VOID_P EQUAL 4) #32 bit
						set( _SUFFIX x86 )
					elseif(CMAKE_SIZEOF_VOID_P EQUAL 8) #64 bit
						set( _SUFFIX x64 )
					endif()
					SET(OPENSSL_ROOT_DIR C:/OpenSSL/openssl-1.1/${_SUFFIX})
				elseif( (CMAKE_SIZEOF_VOID_P EQUAL 4 ) AND ( EXISTS "C:/Program Files (x86)/OpenSSL-Win32" ) )
					SET(OPENSSL_ROOT_DIR "C:/Program Files (x86)/OpenSSL-Win32")
				elseif( (CMAKE_SIZEOF_VOID_P EQUAL 8 ) AND ( EXISTS "C:/Program Files/OpenSSL-Win64/bin" ) )
					SET(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64/bin")
				endif()
				if ( DEFINED OPENSSL_ROOT_DIR )
					MESSAGE( STATUS " Trying ${OPENSSL_ROOT_DIR}" )
					find_package( OpenSSL REQUIRED )
				endif()
			endif()
		endif()
		
		if ( NOT DEFINED OPENSSL_FOUND ) 
			MESSAGE( FATAL_ERROR 
				  "	OPENSSL_ROOT_DIR is not set.\n"
				  " Please set OPENSSL_ROOT_DIR to the root location of your OpenSSL Installation"
				  )
		endif()
		
		MESSAGE( STATUS "OpenSSL version ${OPENSSL_VERSION} Found" )

        if( WIN32 )
            if ( IS_DIRECTORY ${OPENSSL_ROOT_DIR}/bin )
                SET( SUFFIX ".dll" )
                SET( OPENSSL_DEPLOY_BINDIR ${OPENSSL_ROOT_DIR}/bin CACHE INTERNAL "Locations of OpenSSL shared library directory" )
                SET( OPENSSL_DEPLOY_LIBS ${OPENSSL_DEPLOY_BINDIR}/libcrypto-1_1-x64${SUFFIX} ${OPENSSL_DEPLOY_BINDIR}/libssl-1_1-x64${SUFFIX} CACHE INTERNAL "Locations of OpenSSL shared libraries" )
                mark_as_advanced(OPENSSL_DEPLOY_LIBS)
            else()
                MESSAGE( FATAL_ERROR "${OPENSSL_ROOT_DIR}/bin does not exist, so deployment libraries can not be found" )
            endif()
            MESSAGE( STATUS "Checking OpenSSL required shared libraries exist" )
            foreach(lib ${OPENSSL_DEPLOY_LIBS})
                if( NOT EXISTS ${lib} )
                    message( FATAL_ERROR "Could not find OpenSSL shared library '${lib}'" )
                endif()
            endforeach()
            MESSAGE( STATUS "OpenSSL install validated" )
        endif()
	endif()
endfunction()

function(DeployOpenSSL target directory)
    set( options )
    set( oneValueArgs INSTALL_ONLY )
    set( multiValueArgs )

    cmake_parse_arguments( "" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

	if( NOT DEFINED OPENSSL_DEPLOY_LIBS )
		MESSAGE( FATAL_ERROR "Required OpenSSL shared libraries were not found, please call CheckOpenSSL()" )
	endif()
	
	#MESSAGE( STATUS "OpenSSL Found, Deploying OpenSSL Libraries for target '${target}'" )
	#message( STATUS "OPENSSL_LIBRARIES = ${OPENSSL_LIBRARIES}" )
    if( NOT _INSTALL_ONLY )
        foreach(lib ${OPENSSL_DEPLOY_LIBS})
            get_filename_component(filename "${lib}" NAME)
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E echo "Deploying OpenSSL Library '${filename}' for '${target}'"
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${lib}" \"$<TARGET_FILE_DIR:${target}>\"
            )
        endforeach()
    endif()
    
	INSTALL( FILES ${OPENSSL_DEPLOY_LIBS} DESTINATION . )
endfunction()

