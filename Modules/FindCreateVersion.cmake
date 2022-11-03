find_package(InstallFile REQUIRED)

MACRO(CreateVersion dir prefix)

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
    
    set(${prefix}_MAJOR_VERSION ${_CREATE_VERSION_MAJOR})
    set(${prefix}_MINOR_VERSION ${_CREATE_VERSION_MINOR})
    set(${prefix}_PATCH_VERSION ${_CREATE_VERSION_PATCH})
    MATH( EXPR ${prefix}_PATCH_VERSION_LOW  "0x${_CREATE_VERSION_PATCH} & 0x0000FFFF" OUTPUT_FORMAT DECIMAL)
    MATH( EXPR ${prefix}_PATCH_VERSION_HIGH "(0x${_CREATE_VERSION_PATCH} & 0xFFFF0000)>>16" OUTPUT_FORMAT DECIMAL)
    set(${prefix}_DIFF          ${_CREATE_VERSION_DIFF})
    set(${prefix}_APP_NAME      ${_CREATE_VERSION_APP_NAME})
    set(${prefix}_VENDOR        ${_CREATE_VERSION_VENDOR})
    set(${prefix}_HOMEPAGE      ${_CREATE_VERSION_HOMEPAGE})
    set(${prefix}_PRODUCT_HOMEPAGE ${_CREATE_VERSION_PRODUCT_HOMEPAGE})
    set(${prefix}_EMAIL         ${_CREATE_VERSION_EMAIL})
    set(${prefix}_BUILD_DATE    ${_CREATE_VERSION_BUILD_DATE})
    set(${prefix}_BUILD_TIME    ${_CREATE_VERSION_BUILD_TIME})

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
