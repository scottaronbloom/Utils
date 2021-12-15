#include(CMakeParseArguments)

FUNCTION (InstallFile inFile outFile)
    if( IS_DIRECTORY ${outFile} )
        get_filename_component( baseName ${inFile} NAME)
        SET( outFile ${outFile}/${baseName})
    endif()
        
    get_filename_component( baseName ${outFile} NAME)
        
    IF ( EXISTS ${outFile} )
        EXECUTE_PROCESS( 
            COMMAND ${CMAKE_COMMAND} -E compare_files ${inFile} ${outFile} 
            RESULT_VARIABLE filesDifferent
            OUTPUT_QUIET 
            ERROR_QUIET
        )

        IF ( ${filesDifferent} )
            MESSAGE( "${baseName} has been updated." )
            FILE( RENAME ${inFile} ${outFile}  )
        else()
            MESSAGE( "${baseName} is up to date." )
        ENDIF()
    ELSE ()
        MESSAGE( "${baseName} has been updated." )
        FILE( RENAME ${inFile} ${outFile}  )
    ENDIF ()
ENDFUNCTION()

FUNCTION(InstallFilePostBuild)
    set( options )
    set( oneValueArgs TARGET INFILE OUTFILE)
    set( multiValueArgs CONFIGURATIONS )

    cmake_parse_arguments( "" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    if( NOT _TARGET )
        MESSAGE( FATAL_ERROR "TARGET argument not set" )
    ENDIF()

    if( NOT _INFILE )
        MESSAGE( FATAL_ERROR "INFILE argument not set" )
    ENDIF()

    if( NOT _OUTFILE )
        MESSAGE( FATAL_ERROR "OUTFILE argument not set" )
    ENDIF()
    
    SET( _CALL_FUNC_CMD
        ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/SABUtils/Modules/InstallFile.cmake
        --
        ${_INFILE}
        ${_OUTFILE}
    )
    
    if ( _CONFIGURATIONS )
        foreach( currConfig ${_CONFIGURATIONS} )
            add_custom_command( TARGET ${_TARGET} POST_BUILD
                COMMAND "$<$<CONFIG:${currConfig}>:${_CALL_FUNC_CMD}>"
                COMMAND_EXPAND_LISTS
            )
        endforeach()
    else()
        add_custom_command( TARGET ${_TARGET} POST_BUILD
            COMMAND ${_CALL_FUNC_CMD}
            COMMAND_EXPAND_LISTS
        )
     endif()
        
ENDFUNCTION()