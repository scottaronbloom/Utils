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
FUNCTION(VerifyVisualStudio PREFIX)
    SET( ${PREFIX}VISUAL_STUDIO_FOUND FALSE PARENT_SCOPE )
    IF( NOT WIN32)
        return()
    ENDIF()
    
    find_program( CL cl.exe NO_CACHE REQUIRED)
    find_program( LINK link.exe NO_CACHE REQUIRED)
    #find_program( DEVENV devenv.exe NO_CACHE REQUIRED)
    find_program( VSWHERE vswhere.exe 
            PATHS "$ENV{PROGRAMFILES\(X86\)}/Microsoft Visual Studio/Installer"
            NO_CACHE 
            REQUIRED )
    file(TO_CMAKE_PATH ${CL} CL)
    file(TO_CMAKE_PATH ${LINK} LINK)
    #file(TO_CMAKE_PATH ${DEVENV} DEVENV)
    file(TO_CMAKE_PATH ${VSWHERE} VSWHERE)

    #message( STATUS "CL=${CL}" )
    #message( STATUS "LINK=${LINK}" )
    #message( STATUS "DEVENV=${DEVENV}" )
    #message( STATUS "VSWHERE=${VSWHERE}" )
    
    get_filename_component( VSDIR ${CL} DIRECTORY )
    get_filename_component( LNKDIR ${LINK} DIRECTORY )
    #get_filename_component( DEVENVDIR ${DEVENV} DIRECTORY )
    if( NOT ${VSDIR} STREQUAL ${LNKDIR} )
        message( FATAL_ERROR "    cl.exe was found in directory '${VSDIR}'\n    link.exe was found in directory '${LNKDIR}'\nlink.exe and cl.exe should be from the same release" )
    endif()

    get_filename_component( VSARCH ${VSDIR} NAME )
    #message( STATUS "VSARCH=${VSARCH}" )
    if ( ${VSARCH} STREQUAL "x64" )
        SET( VS_OTHER_ARCH "x86" )
    else()
        SET( VS_OTHER_ARCH "x64" )
    endif()
    
    execute_process( 
        COMMAND ${VSWHERE} -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        OUTPUT_VARIABLE installPaths
        ERROR_QUIET
    )
    string( STRIP ${installPaths} installPaths )
    STRING( REPLACE "\n" ";" installPaths ${installPaths} )
    #message( STATUS " installPaths=${installPaths}" )

    execute_process( 
        COMMAND ${VSWHERE} -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property catalog_productLineVersion
        OUTPUT_VARIABLE installProductLineVersions
        ERROR_QUIET
    )
    string( STRIP ${installProductLineVersions} installProductLineVersions )
    STRING( REPLACE "\n" ";" installProductLineVersions ${installProductLineVersions} )
    #message( STATUS " installProductLineVersions=${installProductLineVersions}" )

    execute_process( 
        COMMAND ${VSWHERE} -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion
        OUTPUT_VARIABLE installVersions
        ERROR_QUIET
    )
    string( STRIP ${installVersions} installVersions )
    STRING( REPLACE "\n" ";" installVersionsTmp ${installVersions} )
    SET( installVersions "" )
    #message( STATUS " installVersionsTmp=${installVersionsTmp}" )
    foreach( installVersion ${installVersionsTmp} )
        string( REGEX MATCH "([0-9]+)\.[0-9]+\.[0-9]+(\.[0-9]+)?" TMP ${installVersion} )
        if( CMAKE_MATCH_COUNT EQUAL 0 )
            message( FATAL_ERROR "Could not installVersion's version from ${installVersion}" )
        endif()
        
        LIST( APPEND installVersions ${CMAKE_MATCH_1} )
    endforeach()
    #message( STATUS " installVersions=${installVersions}" )
   
    
    foreach( num IN ZIP_LISTS installPaths installProductLineVersions installVersions )
        file(TO_CMAKE_PATH ${num_0} installPath)
        set(productLineVersion ${num_1})
        set(installVersion ${num_2})

        #message( STATUS "installPath=${installPath}" )
        #message( STATUS "productLineVersion=${productLineVersion}" )
        #message( STATUS "installVersion=${installVersion}" )
        set(vcVerFile ${installPath}/VC/Auxiliary/Build/Microsoft.VCToolsVersion.default.txt)
        if ( EXISTS ${vcVerFile} )
            #message( STATUS "vcVerFile=${vcVerFile}" )
            file( READ ${vcVerFile} vcVer )
            string( STRIP ${vcVer} vcVer )
            #message( STATUS "vcVer='${vcVer}'" )
            SET( vcPath1 ${installPath}/VC/Tools/MSVC/${vcVer}/bin/Host${VSARCH}/${VSARCH}/cl.exe )
            SET( vcPath2 ${installPath}/VC/Tools/MSVC/${vcVer}/bin/Host${VS_OTHER_ARCH}/${VSARCH}/cl.exe )
            if ( EXISTS ${vcPath1} )
                file(TO_CMAKE_PATH ${vcPath1} vcPath)
            elseif( EXISTS ${vcPath2} )
                file(TO_CMAKE_PATH ${vcPath2} vcPath)
            else()
                continue()
            ENDIF()
            
            
            if ( ${vcPath} STREQUAL ${CL} )
                SET( ${PREFIX}VISUAL_STUDIO_FOUND TRUE PARENT_SCOPE )
                SET( ${PREFIX}VISUAL_STUDIO_INSTALLPATH ${installPath} PARENT_SCOPE )
                SET( ${PREFIX}VISUAL_STUDIO_PRODUCTLINEVERSION ${productLineVersion} PARENT_SCOPE )
                SET( ${PREFIX}VISUAL_STUDIO_INSTALLVERSION ${installVersion} PARENT_SCOPE )
                SET( generator "Visual Studio ${installVersion} ${productLineVersion}" )
                if ( ( ${productLineVersion} LESS 2019 ) AND ( ${VSARCH} STREQUAL "x64" ) )
                    SET( generator "${generator} Win64" )
                ENDIF()
                        
                SET( ${PREFIX}VISUAL_STUDIO_GENERATOR ${generator} PARENT_SCOPE )
                if ( NOT "${generator}" STREQUAL "${CMAKE_GENERATOR}" )
                    MESSAGE( FATAL_ERROR "    Path for Visual Studio should use generator '${generator}' is not the same as the current CMAKE_GENERATOR '${CMAKE_GENERATOR}'\n    Please use -G \"${generator}\" or set the environment variable CMAKE_GENERATOR=\"${generator}\"" )
                endif()
                #MESSAGE( STATUS "${PREFIX}VISUAL_STUDIO_INSTALLPATH=${installPath}" )
                #MESSAGE( STATUS "${PREFIX}VISUAL_STUDIO_GENERATOR=${generator}" )
                break()
            endif()
        endif()
    endforeach()
ENDFUNCTION()


