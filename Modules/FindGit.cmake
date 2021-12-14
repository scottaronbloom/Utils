SET(GIT_FOUND FALSE)
SET(GIT_EXE_FOUND FALSE)

FIND_PROGRAM(GIT_EXE_EXECUTABLE git
	DOC "GIT command line client")
MARK_AS_ADVANCED(GIT_EXE_EXECUTABLE)

IF(GIT_EXE_EXECUTABLE)
	SET(GIT_EXE_FOUND TRUE)
	SET(GIT_FOUND TRUE)

	IF( NOT EXISTS "${GIT_EXE_EXECUTABLE}" )
		MESSAGE( "GIT Does not exist: '${GIT_EXE_EXECTUABLE}'" )
		UNSET( GIT_EXE_EXECUTABLE CACHE )
		UNSET( GIT_EXE_FOUND CACHE )
		UNSET( GIT_FOUND CACHE )
		UNSET( GIT_EXE_EXECUTABLE )
		UNSET( GIT_EXE_FOUND )
		UNSET( GIT_FOUND )
		MESSAGE( FATAL_ERROR "GIT Does not exist" )
	ENDIF()

	MACRO(GetGitInfo dir prefix)
		# the GIT commands should be executed with the C locale, otherwise
		# the message (which are parsed) may be translated, Alex
		SET(_GIT_SAVED_LC_ALL "$ENV{LC_ALL}")
		SET(ENV{LC_ALL} C)

		Message( STATUS "Getting GIT info on '${dir}'" )
		Message( STATUS "Using GIT: '${GIT_EXE_EXECUTABLE}'" )
		EXECUTE_PROCESS(
			COMMAND 
				${GIT_EXE_EXECUTABLE} -C "${dir}" 
					log --pretty=format:"%h" -n 1 
					OUTPUT_VARIABLE ${prefix}_REV 
					ERROR_QUIET
					OUTPUT_STRIP_TRAILING_WHITESPACE
		)

		if ("${${prefix}_REV}" STREQUAL "" )
			MESSAGE( WARNING "Could not get GIT info on directory '${dir}'" )
			SET(${prefix}_REV "N/A")
			SET(${prefix}_DIFF "")
			SET(${prefix}_TAG "N/A")
			SET(${prefix}_BRANCH "N/A")
		else()
			execute_process(
				COMMAND ${GIT_EXE_EXECUTABLE} -C "${dir}" 
					diff --quiet --exit-code
					WORKING_DIRECTORY "${dir}"
					RESULT_VARIABLE ${prefix}_DIFF
					ERROR_QUIET
					OUTPUT_STRIP_TRAILING_WHITESPACE
			)
			string(REPLACE "0" "false" ${prefix}_DIFF ${${prefix}_DIFF})
			string(REPLACE "1" "true" ${prefix}_DIFF ${${prefix}_DIFF})

			execute_process(
				COMMAND ${GIT_EXE_EXECUTABLE} -C "${dir}" 
					describe --exact-match --tags
					WORKING_DIRECTORY "${dir}"
					OUTPUT_VARIABLE ${prefix}_TAG
					ERROR_QUIET
					OUTPUT_STRIP_TRAILING_WHITESPACE
			)

			execute_process(
				COMMAND ${GIT_EXE_EXECUTABLE} -C "${dir}" 
					rev-parse --abbrev-ref HEAD
					WORKING_DIRECTORY "${dir}"
					OUTPUT_VARIABLE ${prefix}_TAG
					ERROR_QUIET
					OUTPUT_STRIP_TRAILING_WHITESPACE
			)
		endif()
		SET(ENV{LC_ALL} ${_GIT_SAVED_LC_ALL})
	ENDMACRO()

	MACRO(CreateVersion prefix inFile major minor appName vendor homepage email)

		set(OUTFILE "${CMAKE_BINARY_DIR}/Version.h")
		set(TMP_OUTFILE ${OUTFILE}.tmp)
		
		message( STATUS "Generating version file ${OUTFILE}" )

		set(VERSION_FILE_MAJOR_VERSION ${major})
		set(VERSION_FILE_MINOR_VERSION ${minor})
		set(VERSION_FILE_PATCH_VERSION ${${prefix}_REV})
		set(VERSION_FILE_DIFF          ${${prefix}_DIFF})
		set(VERSION_FILE_APP_NAME      ${appName})
		set(VERSION_FILE_VENDOR        ${vendor})
		set(VERSION_FILE_HOMEPAGE      ${homepage})
		set(VERSION_FILE_EMAIL         ${email})

		configure_file(
			"${inFile}"
			"${TMP_OUTFILE}"
		)

		IF ( EXISTS ${OUTFILE} )
			EXECUTE_PROCESS( 
				COMMAND ${CMAKE_COMMAND} -E compare_files ${TMP_OUTFILE} ${OUTFILE} 
				RESULT_VARIABLE versionChanged
				OUTPUT_QUIET 
				ERROR_QUIET
			)
			IF ( ${versionChanged} )
				FILE( RENAME ${TMP_OUTFILE} ${OUTFILE}  )
			ENDIF()
		ELSE ()
			FILE( RENAME ${TMP_OUTFILE} ${OUTFILE}  )
		ENDIF ()
		
		SET( CMAKE_CONFIGURE_DEPENDS 
			${OUTFILE}
			${CMAKE_CONFIGURE_DEPENDS}
		)
	ENDMACRO()
ENDIF(GIT_EXE_EXECUTABLE)

IF(NOT GIT_FOUND)
  IF(NOT GIT_FIND_QUIETLY)
    MESSAGE(STATUS "GIT executable was not found.")
  ELSE(NOT GIT_FIND_QUIETLY)
    IF(GIT_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "GIT executable was not found.")
    ENDIF(GIT_FIND_REQUIRED)
  ENDIF(NOT GIT_FIND_QUIETLY)
ENDIF(NOT GIT_FOUND)

# FindGIT.cmake ends here.
