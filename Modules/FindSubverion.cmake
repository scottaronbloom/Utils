SET(Subversion_FOUND FALSE)
SET(Subversion_EXE_FOUND FALSE)

FIND_PROGRAM(Subversion_EXE_EXECUTABLE svn
  DOC "subversion command line client")
MARK_AS_ADVANCED(Subversion_EXE_EXECUTABLE)

IF(Subversion_EXE_EXECUTABLE)
  SET(Subversion_EXE_FOUND TRUE)
  SET(Subversion_FOUND TRUE)

  IF( NOT EXISTS "${Subversion_EXE_EXECUTABLE}" )
        MESSAGE( "SVN Does not exist: '${Subversion_EXE_EXECTUABLE}'" )
        UNSET( Subversion_EXE_EXECUTABLE CACHE )
        UNSET( Subversion_EXE_FOUND CACHE )
        UNSET( Subversion_FOUND CACHE )
        UNSET( Subversion_EXE_EXECUTABLE )
        UNSET( Subversion_EXE_FOUND )
        UNSET( Subversion_FOUND )
        MESSAGE( FATAL_ERROR "SVN Does not exist" )
  ENDIF()
        
 
  MACRO(Subversion_GetDate prefix)
    STRING(REGEX REPLACE "^(.*\n)?Last Changed Date: ([0-9][0-9][0-9][0-9])-([0-9][0-9])-([0-9][0-9]).*"
        "\\2" YEAR "${${prefix}_WC_INFO}")
    STRING(REGEX REPLACE "^(.*\n)?Last Changed Date: ([0-9][0-9][0-9][0-9])-([0-9][0-9])-([0-9][0-9]).*"
        "\\3" MONTH   "${${prefix}_WC_INFO}")
    STRING(REGEX REPLACE "^(.*\n)?Last Changed Date: ([0-9][0-9][0-9][0-9])-([0-9][0-9])-([0-9][0-9]).*"
        "\\4" DAY  "${${prefix}_WC_INFO}")

    IF( ${MONTH} MATCHES "01" )
        SET( MONTH "January" )
    ELSEIF( ${MONTH} MATCHES "02" )
        SET( MONTH "February" )
    ELSEIF( ${MONTH} MATCHES "03" )
        SET( MONTH "March" )
    ELSEIF( ${MONTH} MATCHES "04" )
        SET( MONTH "April" )
    ELSEIF( ${MONTH} MATCHES "05" )
        SET( MONTH "May" )
    ELSEIF( ${MONTH} MATCHES "06" )
        SET( MONTH "June" )
    ELSEIF( ${MONTH} MATCHES "07" )
        SET( MONTH "July" )
    ELSEIF( ${MONTH} MATCHES "08" )
        SET( MONTH "August" )
    ELSEIF( ${MONTH} MATCHES "09" )
        SET( MONTH "September" )
    ELSEIF( ${MONTH} MATCHES "10" )
        SET( MONTH "October" )
    ELSEIF( ${MONTH} MATCHES "11" )
        SET( MONTH "November" )
    ELSEIF( ${MONTH} MATCHES "12" )
        SET( MONTH "December" )
    ENDIF()
    SET( ${prefix}_WC_LAST_CHANGED_DATE "${MONTH} ${DAY}, ${YEAR}" )
  ENDMACRO()

  MACRO(Subversion_WC_INFO dir prefix)
    # the subversion commands should be executed with the C locale, otherwise
    # the message (which are parsed) may be translated, Alex
    SET(_Subversion_SAVED_LC_ALL "$ENV{LC_ALL}")
    SET(ENV{LC_ALL} C)

    Message( STATUS "Getting SVN info on '${dir}'" )
    Message( STATUS "Using SVN: '${Subversion_EXE_EXECUTABLE}'" )
    EXECUTE_PROCESS(COMMAND ${Subversion_EXE_EXECUTABLE} --version
      WORKING_DIRECTORY "${dir}"
      OUTPUT_VARIABLE Subversion_VERSION_SVN
      ERROR_VARIABLE Subversion_EXE_info_error
      OUTPUT_STRIP_TRAILING_WHITESPACE)
      
    IF(NOT ${Subversion_EXE_info_result} EQUAL 0)
        MESSAGE(SEND_ERROR "Command \"${Subversion_EXE_EXECUTABLE} --version\" failed with output:\nError:'${Subversion_EXE_info_error}'\nOutput:'${Subversion_EXE_info_result}'")
    ENDIF()


    SET(SVNDIR .)

    EXECUTE_PROCESS(COMMAND ${Subversion_EXE_EXECUTABLE} info ${SVNDIR}
    WORKING_DIRECTORY "${dir}"
      OUTPUT_VARIABLE ${prefix}_WC_INFO
      ERROR_VARIABLE Subversion_EXE_info_error
      RESULT_VARIABLE Subversion_EXE_info_result
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    IF(${Subversion_EXE_info_result} EQUAL 0)
        STRING(REGEX REPLACE "^(.*\n)?svn, version ([.0-9]+).*"
            "\\2" Subversion_VERSION_SVN "${Subversion_VERSION_SVN}")
        STRING(REGEX REPLACE "^(.*\n)?URL: ([^\n]+).*"
            "\\2" ${prefix}_WC_URL "${${prefix}_WC_INFO}")
        STRING(REGEX REPLACE "^(.*\n)?Revision: ([^\n]+).*"
            "\\2" ${prefix}_WC_REVISION "${${prefix}_WC_INFO}")
        STRING(REGEX REPLACE "^(.*\n)?Last Changed Author: ([^\n]+).*"
            "\\2" ${prefix}_WC_LAST_CHANGED_AUTHOR "${${prefix}_WC_INFO}")
        STRING(REGEX REPLACE "^(.*\n)?Last Changed Rev: ([^\n]+).*"
            "\\2" ${prefix}_WC_LAST_CHANGED_REV "${${prefix}_WC_INFO}")
        Subversion_GetDate( ${prefix} )
        STRING(REGEX REPLACE "^.*\\/svn/(.*)(\\/.*)*"
            "\\1" ${prefix}_BUILD_SUFFIX "${${prefix}_WC_URL}")

        IF ( "${${prefix}_BUILD_SUFFIX}" MATCHES ".*releases.*" )
            SET( ${prefix}_BUILD_SUFFIX "" )
        ELSE()
            STRING(REGEX REPLACE "^.*\\/svn\\/(.*)"
                "\\1" ${prefix}_BUILD_SUFFIX "${${prefix}_WC_URL}")
            STRING(REGEX REPLACE "\\/" "_" ${prefix}_BUILD_SUFFIX "${${prefix}_BUILD_SUFFIX}")
            SET( ${prefix}_BUILD_SUFFIX ".${${prefix}_BUILD_SUFFIX}" )
        ENDIF()
    ELSE()
        MESSAGE(SEND_ERROR "Command \"${Subversion_EXE_EXECUTABLE} info . CWD: \"${dir}\" \" failed with output:\nError:'${Subversion_EXE_info_error}'\nOutput:'${Subversion_EXE_info_result}'")
    ENDIF()

    EXECUTE_PROCESS(COMMAND ${Subversion_EXE_EXECUTABLE} propget BluePearlMajorVersion ${SVNDIR}
      WORKING_DIRECTORY "${dir}"
      OUTPUT_VARIABLE ${prefix}_MAJOR_VERSION
      ERROR_VARIABLE Subversion_EXE_info_error
      RESULT_VARIABLE Subversion_EXE_info_result
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    IF(NOT ${Subversion_EXE_info_result} EQUAL 0)
        MESSAGE(SEND_ERROR "Command \"${Subversion_EXE_EXECUTABLE} propget BluePearlMajorVersion ${dir}\" failed with output:\nError:'${Subversion_EXE_info_error}'\nOutput:'${Subversion_EXE_info_result}'")
    ENDIF()


    EXECUTE_PROCESS(COMMAND ${Subversion_EXE_EXECUTABLE} propget BluePearlMinorVersion ${SVNDIR} 
      WORKING_DIRECTORY "${dir}"
      OUTPUT_VARIABLE ${prefix}_MINOR_VERSION
      ERROR_VARIABLE Subversion_EXE_info_error
      RESULT_VARIABLE Subversion_EXE_info_result
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    IF(NOT ${Subversion_EXE_info_result} EQUAL 0)
        MESSAGE(SEND_ERROR "Command \"${Subversion_EXE_EXECUTABLE} propget BluePearlMinorVersion ${dir}\" failed with output:\nError:'${Subversion_EXE_info_error}'\nOutput:'${Subversion_EXE_info_result}'")
    ENDIF()
    SET( ${prefix}_FULL_VERSION ${${prefix}_MAJOR_VERSION}.${${prefix}_MINOR_VERSION}.${${prefix}_WC_LAST_CHANGED_REV}${${prefix}_BUILD_SUFFIX} )
    # restore the previous LC_ALL
    SET(ENV{LC_ALL} ${_Subversion_SAVED_LC_ALL})

  ENDMACRO(Subversion_WC_INFO)

  MACRO(Subversion_WC_LOG dir prefix)
    # This macro can block if the certificate is not signed:
    # svn ask you to accept the certificate and wait for your answer
    # This macro requires a svn server network access (Internet most of the time)
    # and can also be slow since it access the svn server
    SET(SVNDIR .)

    EXECUTE_PROCESS(COMMAND
      ${Subversion_EXE_EXECUTABLE} log -r BASE ${SVNDIR}
      WORKING_DIRECTORY "${dir}"
      OUTPUT_VARIABLE ${prefix}_LAST_CHANGED_LOG
      ERROR_VARIABLE Subversion_EXE_log_error
      RESULT_VARIABLE Subversion_EXE_log_result
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    IF(NOT ${Subversion_EXE_log_result} EQUAL 0)
      MESSAGE(SEND_ERROR "Command \"${Subversion_EXE_EXECUTABLE} log -r BASE ${dir}\" failed with output:\nError:'${Subversion_EXE_info_error}'\nOutput:'${Subversion_EXE_info_result}'")
    ENDIF(NOT ${Subversion_EXE_log_result} EQUAL 0)
  ENDMACRO(Subversion_WC_LOG)

ENDIF(Subversion_EXE_EXECUTABLE)

IF(NOT Subversion_FOUND)
  IF(NOT Subversion_FIND_QUIETLY)
    MESSAGE(STATUS "Subversion executable was not found.")
  ELSE(NOT Subversion_FIND_QUIETLY)
    IF(Subversion_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Subversion executable was not found.")
    ENDIF(Subversion_FIND_REQUIRED)
  ENDIF(NOT Subversion_FIND_QUIETLY)
ENDIF(NOT Subversion_FOUND)

# FindSubversion.cmake ends here.
