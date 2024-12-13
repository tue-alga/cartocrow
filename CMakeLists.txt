cmake_minimum_required(VERSION 3.15)

project(cartocrow
    VERSION 0.1.5
    DESCRIPTION "Framework for algorithmic thematic mapping"
    HOMEPAGE_URL "https://github.com/tue-alga/cartocrow"
    LANGUAGES CXX)

set(CMAKE_INSTALL_BINARY_DIR bin CACHE STRING "Installation subdirectory for compiled executables and shared libraries.")
set(CMAKE_INSTALL_LIBRARY_DIR lib CACHE STRING "Installation subdirectory for compiled static libraries.")
set(CMAKE_INSTALL_SOURCE_DIR include CACHE STRING "Installation subdirectory for source code.")
set(CMAKE_INSTALL_SCRIPT_DIR script CACHE STRING "Installation subdirectory for scripts.")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(INSTALL_CONFIG_DIR ${CMAKE_INSTALL_LIBRARY_DIR}/cmake)
set(INSTALL_BINARY_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINARY_DIR})
set(INSTALL_SCRIPT_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SCRIPT_DIR})

macro(install_files_and_directories FILE_LIST ROOT_DIR)
    foreach(FILE ${FILE_LIST})
        get_filename_component(DIR ${FILE} DIRECTORY)
        string(REGEX REPLACE "^${CMAKE_CURRENT_SOURCE_DIR}" "" DIR ${DIR})
        install(FILES ${FILE} DESTINATION "${ROOT_DIR}/${DIR}")
    endforeach(FILE)
endmacro(install_files_and_directories)

# Indicate the version to CMake (for find_package)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    CartoCrowConfigVersion.cmake
    VERSION 0.1.5
    COMPATIBILITY AnyNewerVersion
)

if(MSVC)
    add_compile_definitions(_USE_MATH_DEFINES)
endif()

# ## DEPENDENCIES ###
find_package(CGAL REQUIRED)
find_path(CGAL_INCLUDE_DIR CGAL/Exact_predicates_inexact_constructions_kernel.h)
include_directories(${CGAL_INCLUDE_DIR})
link_libraries(${CGAL_LIBRARIES})
find_package(GMP REQUIRED)

find_package(GDAL CONFIG REQUIRED)
find_package(Qt5Widgets REQUIRED)
set(CMAKE_AUTOMOC ON)
find_package(glog REQUIRED)
find_package(Ipelib REQUIRED)

# ## SOURCES ###

# All source files should use include paths relative to the source root
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_subdirectory(cartocrow)
add_subdirectory(demos)
add_subdirectory(frontend)
add_subdirectory(test)

# Install rules
install(
    FILES
    "${CMAKE_SOURCE_DIR}/cmake/CartoCrowConfig.cmake"
    "${CMAKE_SOURCE_DIR}/cmake/FindIpelib.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/CartoCrowConfigVersion.cmake"
    DESTINATION ${INSTALL_CONFIG_DIR}/CartoCrow
)
