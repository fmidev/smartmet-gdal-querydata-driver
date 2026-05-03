cmake_minimum_required(VERSION 3.7.1)
project(MacGyver)

set(CMAKE_MFC_FLAG 0)

file( 
    GLOB SOURCE_FILES 
    macgyver/* 
    )

list(
    REMOVE_ITEM SOURCE_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/macgyver/TemplateFormatter.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/macgyver/TemplateFormatter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/macgyver/TemplateFormatterMT.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/macgyver/TemplateFormatterMT.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/macgyver/TemplateDirectory.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/macgyver/TemplateDirectory.cpp"
    )

add_library( 
    ${PROJECT_NAME} STATIC
    ${SOURCE_FILES} 
    )

target_link_libraries( 
    ${PROJECT_NAME}
    fmt
    )

target_include_directories( 
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PUBLIC ${PROJECT_SOURCE_DIR}
    PUBLIC ${BOOST_INCLUDE_DIR}
    )

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/macgyver
    ${PROJECT_SOURCE_DIR}
    ${BOOST_INCLUDE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )
