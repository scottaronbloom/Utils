SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_MODULE_PATH})

find_package( InstallFile REQUIRED )

set(INFILE_FOUND false)
set(OUTFILE_FOUND false)
set(DASH_FOUND false)
MATH(EXPR _END "${CMAKE_ARGC}-1")
foreach(_arg RANGE 1 ${_END})
    SET(currArgV ${CMAKE_ARGV${_arg}})
    
    #message( STATUS "_arg=${_arg}" )
    #message( STATUS "currArgV=${currArgV}" )
    if ( DASH_FOUND )
        IF ( INFILE_FOUND AND OUTFILE_FOUND )
            break()
        ELSEIF( INFILE_FOUND AND NOT OUTFILE_FOUND )
            #MESSAGE( STATUS "Out found" )
            SET(OUTFILE_FOUND true)
            SET(OUTFILE ${currArgV})
        ELSEIF( NOT INFILE_FOUND AND NOT OUTFILE_FOUND )
            #MESSAGE( STATUS "In found" )
            SET(INFILE_FOUND true)
            SET(INFILE ${currArgV})
        ENDIF()
    elseif ( currArgV STREQUAL "--" )
        #MESSAGE( STATUS "Dash found" )
        SET(DASH_FOUND true)
    endif()
endforeach()


#MESSAGE( STATUS "INFILE_FOUND=${INFILE_FOUND}" )
#MESSAGE( STATUS "OUTFILE_FOUND=${OUTFILE_FOUND}" )
#MESSAGE( STATUS "DASH_FOUND=${DASH_FOUND}" )

#MESSAGE( STATUS "INFILE=${INFILE}" )
#MESSAGE( STATUS "OUTFILE=${OUTFILE}" )

IF ( NOT INFILE_FOUND )
    message( FATAL_ERROR " Usage: -P ${CMAKE_CURRENT_LIST_FILE} -- <infile> <outfile>\n Missing <infile> argument." )
endif()

IF ( NOT OUTFILE_FOUND )
    message( FATAL_ERROR " Usage: -P ${CMAKE_CURRENT_LIST_FILE} -- <infile> <outfile>\n Missing <outfile> argument." )
endif()

InstallFile( ${INFILE} ${OUTFILE} )