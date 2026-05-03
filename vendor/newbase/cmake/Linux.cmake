message(STATUS "System Name: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Install Prefix: ${CMAKE_INSTALL_PREFIX}")

option(BUILD_SHARED_LIBS "Build shared libs" ON)
option(DISABLE_GDAL "Disable GDAL" OFF)

set(CMAKE_CXX_FLAGS "-DUNIX -D_REENTRANT")

if(DISABLE_GDAL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDISABLED_GDAL")
    unset(GDAL_LIBRARY CACHE)
    unset(GDAL_INCLUDE_DIR CACHE)
    unset(GDAL_CONFIG CACHE)
else()
    find_package(GDAL REQUIRED)
endif()

find_package(Boost 1.55.0 REQUIRED COMPONENTS regex thread filesystem system date_time)

add_compile_options(-std=c++11 -fPIC -MD -Wall -W -Wno-unused-parameter)

find_package(Fmt REQUIRED)
if (Fmt_FOUND)
  if (BUILD_SHARED_LIBS)
    find_library(Fmt_SHARED_LIB fmt ${CMAKE_INSTALL_PREFIX} PATH_SUFFIXES so)
    set(Fmt_LIBRARIES ${Fmt_SHARED_LIB})
  endif()
  if (Fmt_LIBRARIES-NOTFOUND)
    message(FATAL_ERROR "Fmt library not found")
  endif()
  message(STATUS "Fmt: ${Fmt_LIBRARIES}")
endif()
