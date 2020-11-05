cmake_minimum_required(VERSION 3.13.0)

# GeoViz project.
set(PROJECT_NAME GeoViz)
project(${PROJECT_NAME})

# CMake options.
set(CMAKE_INSTALL_BINARY_DIR bin CACHE STRING "Installation subdirectory for compiled executables and shared libraries.")
set(CMAKE_INSTALL_LIBRARY_DIR lib CACHE STRING "Installation subdirectory for compiled static libraries.")
set(CMAKE_INSTALL_SOURCE_DIR include CACHE STRING "Installation subdirectory for source code.")
set(CMAKE_INSTALL_SCRIPT_DIR script CACHE STRING "Installation subdirectory for scripts.")

option(GEOVIZ_CREATE_ARCHIVE "Create an archive GeoViz library, collecting all Geo-visualization methods." ON)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(INSTALL_CONFIG_DIR ${CMAKE_INSTALL_LIBRARY_DIR}/cmake)
set(INSTALL_BINARY_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINARY_DIR})
set(INSTALL_SCRIPT_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SCRIPT_DIR})

# Macros.
macro(install_files_and_directories FILE_LIST ROOT_DIR)
    foreach(FILE ${FILE_LIST})
        get_filename_component(DIR ${FILE} DIRECTORY)
        string(REGEX REPLACE "^${CMAKE_CURRENT_SOURCE_DIR}" "" DIR ${DIR})
        INSTALL(FILES ${FILE} DESTINATION "${ROOT_DIR}/${DIR}")
    endforeach(FILE)
endmacro(install_files_and_directories)

# Project version.
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_FEATURE 5)
set(VERSION_PATCH 0)
set(VERSION_PROJECT "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_FEATURE}.${VERSION_PATCH}")
message(STATUS "${PROJECT_NAME} version: ${VERSION_PROJECT}")

# Indicate the version to CMake (for find_package)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    GeoVizConfigVersion.cmake
    VERSION ${VERSION_PROJECT}
    COMPATIBILITY AnyNewerVersion
)

# Configure a header file to pass CMake settings to the source code.
configure_file(
    "${CMAKE_MODULE_PATH}/geoviz_config.h.in"
    "${CMAKE_MODULE_PATH}/geoviz_config.h"
)
install(FILES "${CMAKE_MODULE_PATH}/geoviz_config.h" DESTINATION "${CMAKE_INSTALL_SOURCE_DIR}/cmake")

# Configure the doxygen configuration file to pass CMake settings to the doxygen.
configure_file(
    "${CMAKE_MODULE_PATH}/Doxyfile.in"
    "Doxyfile"
)


### DEPENDENCIES ###

find_package(CGAL)


### SOURCES ###

# All source files should use include paths relative to the source root.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# By default, we use the C++14 dialect (required by CGAL).
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The GeoViz library.
add_subdirectory(geoviz)

# Executables that use or expose GeoViz.
add_subdirectory(console)

# Unit tests of the GeoViz library.
add_subdirectory(test)


# Install rules.
install(
    FILES
    "${CMAKE_MODULE_PATH}/GeoVizConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/GeoVizConfigVersion.cmake"
    DESTINATION ${INSTALL_CONFIG_DIR}/GeoViz
)

# Custom install target for IDE.
# Removed, because the install prefix is not propagated to the dependencies,
# meaning that they will always try to install to the system directory.
# So far, I have not found a way to check whether the project was created by an IDE or from the console or the CMake GUI,
# which could then be used to manually set the install prefix.
#set(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_SOURCE_DIR}")
#add_custom_target(
#    install_${PROJECT_NAME}
#    "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target install
#    DEPENDS ${GEOVIZ_TARGET}
#    COMMENT "Installing ${PROJECT_NAME}"
#)
