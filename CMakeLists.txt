cmake_minimum_required(VERSION 3.15)

project(CartoCrow
    VERSION 0.1.5
    DESCRIPTION "Framework for algorithmic thematic mapping"
    HOMEPAGE_URL "https://github.com/tue-alga/cartocrow"
    LANGUAGES CXX)

# CMake options.
set(CMAKE_INSTALL_BINARY_DIR bin CACHE STRING "Installation subdirectory for compiled executables and shared libraries.")
set(CMAKE_INSTALL_LIBRARY_DIR lib CACHE STRING "Installation subdirectory for compiled static libraries.")
set(CMAKE_INSTALL_SOURCE_DIR include CACHE STRING "Installation subdirectory for source code.")
set(CMAKE_INSTALL_SCRIPT_DIR script CACHE STRING "Installation subdirectory for scripts.")

option(CARTOCROW_CREATE_ARCHIVE "Create an archive CartoCrow library, collecting all Geo-visualization methods." OFF)

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
    CartoCrowConfigVersion.cmake
    VERSION ${VERSION_PROJECT}
    COMPATIBILITY AnyNewerVersion
)

# Configure a header file to pass CMake settings to the source code.
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/cartocrow_config.h.in"
    "${CMAKE_SOURCE_DIR}/cmake/cartocrow_config.h"
)
install(FILES "${CMAKE_SOURCE_DIR}/cmake/cartocrow_config.h" DESTINATION "${CMAKE_INSTALL_SOURCE_DIR}/cmake")

# Configure the doxygen configuration file to pass CMake settings to the doxygen.
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/Doxyfile.in"
    "Doxyfile"
)


### DEPENDENCIES ###

find_package(CGAL REQUIRED)
find_path(CGAL_INCLUDE_DIR CGAL/Exact_predicates_inexact_constructions_kernel.h)
include_directories(${CGAL_INCLUDE_DIR})

find_package(gflags REQUIRED)
find_package(glog REQUIRED)
find_package(tinyxml2 REQUIRED)


### SOURCES ###

# All source files should use include paths relative to the source root
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The CartoCrow library
add_subdirectory(cartocrow)

# Executables that use or expose CartoCrow
add_subdirectory(console)

# Unit tests of the CartoCrow library
add_subdirectory(test)


# Install rules
install(
    FILES
    "${CMAKE_SOURCE_DIR}/cmake/CartoCrowConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/CartoCrowConfigVersion.cmake"
    DESTINATION ${INSTALL_CONFIG_DIR}/CartoCrow
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
#    DEPENDS ${CARTOCROW_TARGET}
#    COMMENT "Installing ${PROJECT_NAME}"
#)
