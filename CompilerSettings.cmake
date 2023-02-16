# The MIT License( MIT )
#
# Copyright( c ) 2020-2021 Scott Aron Bloom
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files( the "Software" ), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions :
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

IF(CMAKE_SIZEOF_VOID_P EQUAL 4) #32 bit
    SET( BITSIZE 32 )
ELSEIF(CMAKE_SIZEOF_VOID_P EQUAL 8) #64 bit
    SET( BITSIZE 64 )
    add_definitions( -D_AMD64_ )
ELSE () 
    MESSAGE( STATUS "Unknown Bitsize - CMAKE_SIZEOF_VOID_P not set to 4 or 8" )
    MESSAGE( STATUS "-DCMAKE_SIZEOF_VOID_P=4 for 32 bit" )
    MESSAGE( FATAL_ERROR "-DCMAKE_SIZEOF_VOID_P=8 for 64 bit" )
ENDIF() 

IF(WIN32)
    IF( ${BITSIZE} EQUAL 32 )
        SET(CMAKE_CXX_FLAGS_DEBUG          "/EHsc /MP /D_DEBUG /RTC1 /MDd /ZI /W3 /Od             /w34700 /w34701 /w34715 /w34716 /w34717 /w34062 /w44800 /wd4251 /wd4231 /wd4503")
        SET(CMAKE_C_FLAGS_DEBUG            "/EHsc /MP /D_DEBUG /RTC1 /MDd /ZI /W3 /Od             /w34700 /w34701 /w34715 /w34716 /w34717 /w34062 /w44800 /wd4251 /wd4231 /wd4503")
        SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/EHsc /MP /MD /Zi /O2 /Ob1 /DNDEBUG                       /w34700 /w34701 /w34715 /w34716 /w34717 /w34062 /w44800 /wd4251 /wd4231 /wd4503")
        SET(CMAKE_C_FLAGS_RELWITHDEBINFO   "/EHsc /MP /MD /Zi /O2 /Ob1 /DNDEBUG                       /w34700 /w34701 /w34715 /w34716 /w34717 /w34062 /w44800 /wd4251 /wd4231 /wd4503")
        SET(CMAKE_CXX_FLAGS_RELEASE        "/EHsc /MP /MD /Zi /O2 /Ob1 /DNDEBUG                       /w34700 /w34701 /w34715 /w34716 /w34717 /w34062 /w44800 /wd4251 /wd4231 /wd4503")
        SET(CMAKE_C_FLAGS_RELEASE          "/EHsc /MP /MD /Zi /O2 /Ob1 /DNDEBUG                       /w34700 /w34701 /w34715 /w34716 /w34717 /w34062 /w44800 /wd4251 /wd4231 /wd4503")
        SET(CMAKE_CXX_FLAGS_MINSIZEREL     "/EHsc /MP /MD /Zi /O2 /Ob1 /DNDEBUG                       /w34700 /w34701 /w34715 /w34716 /w34717 /w34062 /w44800 /wd4251 /wd4231 /wd4503")
        SET(CMAKE_C_FLAGS_MINSIZEREL       "/EHsc /MP /MD /Zi /O2 /Ob1 /DNDEBUG                       /w34700 /w34701 /w34715 /w34716 /w34717 /w34062 /w44800 /wd4251 /wd4231 /wd4503")
    ELSE()
        SET(CMAKE_CXX_FLAGS_DEBUG          "/EHsc /MP /D_DEBUG /RTC1 /MDd /ZI /W3 /Od /bigobj     /w34700 /w34701 /w34715 /w34716 /w34717 /w34062 /w44800 /wd4251 /wd4231 /wd4503")
        SET(CMAKE_C_FLAGS_DEBUG            "/EHsc /MP /D_DEBUG /RTC1 /MDd /ZI /W3 /Od /bigobj     /w34700 /w34701 /w34715 /w34716 /w34717 /w34062 /w44800 /wd4251 /wd4231 /wd4503")

        SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/EHsc /MP /DNDEBUG       /MD /Zi /O2 /Ob1 /bigobj     /w34700 /w34701 /w34715 /w34716 /w34717 /w44800 /w34062 /wd4251 /wd4231 /wd4503")
        SET(CMAKE_C_FLAGS_RELWITHDEBINFO   "/EHsc /MP /DNDEBUG       /MD /Zi /O2 /Ob1 /bigobj     /w34700 /w34701 /w34715 /w34716 /w34717 /w44800 /w34062 /wd4251 /wd4231 /wd4503")
        SET(CMAKE_CXX_FLAGS_RELEASE        "/EHsc /MP /DNDEBUG       /MD /Zi /O2 /Ob1 /bigobj     /w34700 /w34701 /w34715 /w34716 /w34717 /w44800 /w34062 /wd4251 /wd4231 /wd4503")
        SET(CMAKE_C_FLAGS_RELEASE          "/EHsc /MP /DNDEBU        /MD /Zi /O2 /Ob1 /bigobj     /w34700 /w34701 /w34715 /w34716 /w34717 /w44800 /w34062 /wd4251 /wd4231 /wd4503")
        SET(CMAKE_CXX_FLAGS_MINSIZEREL     "/EHsc /MP /DNDEBUG       /MD /Zi /O2 /Ob1 /bigobj     /w34700 /w34701 /w34715 /w34716 /w34717 /w44800 /w34062 /wd4251 /wd4231 /wd4503")
        SET(CMAKE_C_FLAGS_MINSIZEREL       "/EHsc /MP /DNDEBUG       /MD /Zi /O2 /Ob1 /bigobj     /w34700 /w34701 /w34715 /w34716 /w34717 /w44800 /w34062 /wd4251 /wd4231 /wd4503")

	    SET(CMAKE_EXE_LINKER_FLAGS_DEBUG          "${CMAKE_EXE_LINKER_FLAGS_DEBUG}          /STACK:18388608 /HIGHENTROPYVA:NO")
	    SET(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /STACK:18388608 /HIGHENTROPYVA:NO")   
	    SET(CMAKE_EXE_LINKER_FLAGS_RELEASE        "${CMAKE_EXE_LINKER_FLAGS_RELEASE}        /STACK:18388608 /HIGHENTROPYVA:NO")   
	    SET(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL     "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL}     /STACK:18388608 /HIGHENTROPYVA:NO")   
    ENDIF()

    IF(NOT NO_WARNING_AS_ERROR)
        SET(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} /WX /w34100")
        SET(CMAKE_C_FLAGS_DEBUG            "${CMAKE_C_FLAGS_DEBUG} /WX /w34100")
        SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /WX /w34100")
        SET(CMAKE_C_FLAGS_RELWITHDEBINFO   "${CMAKE_C_FLAGS_RELWITHDEBINFO} /WX /w34100")
        SET(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} /WX /w34100")
        SET(CMAKE_C_FLAGS_RELEASE          "${CMAKE_C_FLAGS_RELEASE} /WX /w34100")
        SET(CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL} /WX /w34100")
        SET(CMAKE_C_FLAGS_MINSIZEREL       "${CMAKE_C_FLAGS_MINSIZEREL} /WX /w34100")
    ENDIF()

    IF( WARN_ALL ) 
        SET(CMAKE_CXX_FLAGS_DEBUG        "${CMAKE_CXX_FLAGS_DEBUG} /W4")
        SET(CMAKE_C_FLAGS_DEBUG          "${CMAKE_C_FLAGS_DEBUG} /W4")
        SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /W4")
        SET(CMAKE_C_FLAGS_RELWITHDEBINFO   "${CMAKE_C_FLAGS_RELWITHDEBINFO} /W4")
        SET(CMAKE_CXX_FLAGS_RELEASE      "${CMAKE_CXX_FLAGS_RELEASE} /W4")
        SET(CMAKE_C_FLAGS_RELEASE        "${CMAKE_C_FLAGS_RELEASE} /W4")
        SET(CMAKE_CXX_FLAGS_MINSIZEREL   "${CMAKE_CXX_FLAGS_MINSIZEREL} /W4")
        SET(CMAKE_C_FLAGS_MINSIZEREL     "${CMAKE_C_FLAGS_MINSIZEREL} /W4")
    ENDIF()
ELSE()
    IF(NOT NO_EXTENSIVE_WARNINGS)
        # the following are from -Wall
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Waddress" )
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wchar-subscripts" )

        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wcomment" )
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wformat" )

        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wmaybe-uninitialized" )
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wmissing-braces" )
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wnonnull" )
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wparentheses" )
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wreturn-type")
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wsequence-point")
        #SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wsign-compare") # big changes

        # we dont use -fstrict aliasing or -fstrict-overflow the following 2 are worthless
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wstrict-aliasing" )
        #SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wstrict-overflow=1" )

        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wswitch" )
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wtrigraphs" )
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wuninitialized" )
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wunknown-pragmas" ) # have to figure out a way with windows
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wunused-function" )
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wunused-label" )
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wunused-value" )
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wunused-variable" )
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wvolatile-register-var" )

        #the following are form -Wextra
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wclobbered" )
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wempty-body" )
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wignored-qualifiers" )
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wmissing-field-initializers" )
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wtype-limits" )
        #SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wshift-negative-value" ) #not available on our compiler
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wunused-parameter" )
        SET(C_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wunused-but-set-parameter" )

        SET(CXX_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wno-return-local-addr")
        SET(CXX_EXTENSIVE_WARNINGS "${C_EXTENSIVE_WARNINGS} -Wreorder")
        SET(CXX_EXTENSIVE_WARNINGS "${CXX_EXTENSIVE_WARNINGS} -Wc++11-compat" )
        #SET(CXX_EXTENSIVE_WARNINGS "${CXX_EXTENSIVE_WARNINGS} -Werror=suggest-override" )
        #SET(CXX_EXTENSIVE_WARNINGS "${CXX_EXTENSIVE_WARNINGS} -Wc++14-compat" ) # not available on our compiler
        SET(CMAKE_CXX_FLAGS    "${CMAKE_CXX_FLAGS} ${CXX_EXTENSIVE_WARNINGS}" )
        SET(CMAKE_C_FLAGS      "${CMAKE_C_FLAGS} ${C_EXTENSIVE_WARNINGS}" )
    ELSE()
        MESSAGE( WARNING "Extensive warnings disabled - ${PROJECT_NAME}" )
    ENDIF()
    IF(NOT NO_WARNING_AS_ERROR)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
    ELSE()
        MESSAGE( WARNING "Warning as an Error disabled - ${PROJECT_NAME}" )
    ENDIF()

    IF( WARN_ALL )
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
        MESSAGE( STATUS  "Warning all enabled - ${CMAKE_PROJECT_NAME}" )
    ENDIF()

    set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} -fPIC")
ENDIF()

IF(WIN32)
        add_definitions(
               -D_CRT_SECURE_NO_WARNINGS
               -D_CRT_SECURE_NO_DEPRECATE
               -D_CRT_NONSTDC_NO_WARNINGS
               -D_SCL_SECURE_NO_WARNINGS
        )
		IF( ${BITSIZE} EQUAL 32 )
			if ( ${MSVC_VERSION} GREATER_EQUAL  1800 )
				add_link_options( "/SAFESEH:NO" )
			endif()
		ENDIF()
ENDIF()

add_definitions( -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS  )

add_definitions( -DUNICODE )
if ( SABUTILS_DLL )
    add_definitions( -DSABUTILS_DLL )
endif()

IF( SAB_DEBUG_TRACE )
    add_definitions( -DSAB_DEBUG_TRACE )
ENDIF()
