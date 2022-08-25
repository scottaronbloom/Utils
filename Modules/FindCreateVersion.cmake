find_package(InstallFile REQUIRED)

MACRO(CreateVersion dir)

    set( options )
    set( oneValueArgs MAJOR MINOR PATCH DIFF APP_NAME VENDOR HOMEPAGE PRODUCT_HOMEPAGE EMAIL BUILD_DATE BUILD_TIME)
    set( multiValueArgs )
    cmake_parse_arguments(
        _CREATE_VERSION
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
        )

    set( IN_FILE ${CMAKE_SOURCE_DIR}/SABUtils/Modules/Version.h.in )

    set(OUTFILE "${CMAKE_BINARY_DIR}/Version.h")
    set(TMP_OUTFILE ${OUTFILE}.tmp)
    
    #message( STATUS "_CREATE_VERSION_MAJOR=${_CREATE_VERSION_MAJOR}" )
    #message( STATUS "_CREATE_VERSION_MINOR=${_CREATE_VERSION_MINOR}" )
    #message( STATUS "_CREATE_VERSION_PATCH=${_CREATE_VERSION_PATCH}" )
    #message( STATUS "_CREATE_VERSION_DIFF=${_CREATE_VERSION_DIFF}" )
    message( STATUS "Generating version file '${OUTFILE}'" )
    
    set(VERSION_FILE_MAJOR_VERSION ${_CREATE_VERSION_MAJOR})
    set(VERSION_FILE_MINOR_VERSION ${_CREATE_VERSION_MINOR})
    set(VERSION_FILE_PATCH_VERSION ${_CREATE_VERSION_PATCH})
    set(VERSION_FILE_DIFF          ${_CREATE_VERSION_DIFF})
    set(VERSION_FILE_APP_NAME      ${_CREATE_VERSION_APP_NAME})
    set(VERSION_FILE_VENDOR        ${_CREATE_VERSION_VENDOR})
    set(VERSION_FILE_HOMEPAGE      ${_CREATE_VERSION_HOMEPAGE})
    set(VERSION_FILE_PRODUCT_HOMEPAGE ${_CREATE_VERSION_PRODUCT_HOMEPAGE})
    set(VERSION_FILE_EMAIL         ${_CREATE_VERSION_EMAIL})
    set(VERSION_FILE_BUILD_DATE    ${_CREATE_VERSION_BUILD_DATE})
    set(VERSION_FILE_BUILD_TIME    ${_CREATE_VERSION_BUILD_TIME})

    configure_file(
        "${IN_FILE}"
        "${TMP_OUTFILE}"
    )

    InstallFile( ${TMP_OUTFILE} ${OUTFILE} REMOVE_ORIG ) # creates a dependency on TMP_OUTFILE
    set_property( 
        DIRECTORY ${dir} 
        APPEND
        PROPERTY CMAKE_CONFIGURE_DEPENDS
        ${OUTFILE}
        )

ENDMACRO()
