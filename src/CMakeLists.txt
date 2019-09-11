cmake_minimum_required (VERSION 2.6)

# The CMake install prefix default is changed to reflect
# the normal place to install the software to be used by the website.
set(INSTALL_ROOT ${CMAKE_BINARY_DIR}/../wwwroot)
set(INSTALL_MESSAGE "Install path prefix, prepended onto install directories.")
set(CMAKE_INSTALL_PREFIX ${INSTALL_ROOT} CACHE PATH "${INSTALL_MESSAGE}")

set(CMAKE_INSTALL_BINARY_DIR bin CACHE STRING "Installation subdirectory for compiled executables and shared libraries.")
set(CMAKE_INSTALL_LIBRARY_DIR lib CACHE STRING "Installation subdirectory for compiled static libraries.")
set(CMAKE_INSTALL_SOURCE_DIR include CACHE STRING "Installation subdirectory for source code.")

# GeoViz project.
set(PROJECT_NAME GeoViz)
project(${PROJECT_NAME})

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${INSTALL_ROOT} CACHE PATH "${INSTALL_MESSAGE}")
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# CMake options.
option(GEOVIZ_CREATE_ARCHIVE "Create an archive GeoViz library, collecting all Geo-visualization methods." ON)

# Macros.
macro(install_files_and_directories FILE_LIST ROOT_DIR)
    foreach(FILE ${FILE_LIST})
        get_filename_component(DIR ${FILE} DIRECTORY)
        string(REGEX REPLACE "^${CMAKE_CURRENT_SOURCE_DIR}" "" DIR ${DIR})
        INSTALL(FILES ${FILE} DESTINATION "${ROOT_DIR}/${DIR}")
    endforeach(FILE)
endmacro(install_files_and_directories)

# Set version numbers.
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_FEATURE 0)
set(VERSION_PATCH 0)
set(VERSION_PROJECT "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_FEATURE}.${VERSION_PATCH}")
message(STATUS "${PROJECT_NAME} version: ${VERSION_PROJECT}")

# Configure a header file to pass CMake settings to the source code.
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/geoviz_config.h.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/geoviz_config.h"
)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake/geoviz_config.h" DESTINATION "${CMAKE_INSTALL_SOURCE_DIR}/cmake")

# Indicate the version to CMake (for find_package)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    GeoVizConfigVersion.cmake
    VERSION ${VERSION_PROJECT}
    COMPATIBILITY AnyNewerVersion
)

# All source files should use include paths relative to the source root.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})


# The GeoViz library.
add_subdirectory(geoviz)

# Executables that use or expose GeoViz.
add_subdirectory(console)

install(
    FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GeoVizConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/GeoVizConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}/cmake/GeoViz
)