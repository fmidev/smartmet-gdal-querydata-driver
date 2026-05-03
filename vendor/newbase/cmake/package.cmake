set(PACKAGE_VERSION "18.1.4")
set(CPACK_PACKAGE_NAME "smartmet-library-${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION ${PACKAGE_VERSION})
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")


macro(cpack_optional_append _list _cond _item)
  if(${_cond})
    set(${_list} ${${_list}} ${_item})
  endif()
endmacro()

option(CPACK_BINARY_RPM  "Enable to build RPM package" ON)

cpack_optional_append(CPACK_SOURCE_GENERATOR TRUE TGZ)
cpack_optional_append(CPACK_GENERATOR CPACK_BINARY_RPM RPM)

set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL libraries headers)

if(CPACK_BINARY_RPM)
    option(CPACK_RPM_PACKAGE_DEBUG "Enable RPM package debug" OFF)

    set(CPACK_RPM_PACKAGE_NAME "${CPACK_PACKAGE_NAME}")
    if(NOT BUILD_SHARED_LIBS)
        set(CPACK_RPM_PACKAGE_NAME "${CPACK_RPM_PACKAGE_NAME}-static")
    endif()
    set(CPACK_RPM_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}")
    set(CPACK_RPM_PACKAGE_RELEASE "${CPACK_PACKAGE_RELEASE}")
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
    set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")
    set(CPACK_RPM_PACKAGE_VENDOR "Finnish Meteorological Institute")
    set(CPACK_RPM_PACKAGE_URL "https://github.com/fmidev/smartmet-library-newbase")
    set(CPACK_RPM_PACKAGE_CONTACT "opendata@fmi.fi")
    set(CPACK_RPM_PACKAGE_DESCRIPTION "FMI newbase library")
    set(CPACK_RPM_PACKAGE_SUMMARY "newbase library")

    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/smartmet-library-newbase.spec.in" "${CMAKE_CURRENT_BINARY_DIR}/smartmet-library-newbase.spec" @ONLY IMMEDIATE)
    set(CPACK_RPM_USER_BINARY_SPECFILE "${CMAKE_CURRENT_BINARY_DIR}/smartmet-library-newbase.spec")
endif()

include(CPack)

