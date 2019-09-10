cmake_minimum_required (VERSION 2.6)

# The CMake install prefix default is changed to reflect
# the normal place to install the software to be used by the website.
set(INSTALL_ROOT ${CMAKE_BINARY_DIR}/../wwwroot)
set(INSTALL_MESSAGE "Install path prefix, prepended onto install directories.")
set(CMAKE_INSTALL_PREFIX ${INSTALL_ROOT} CACHE PATH "${INSTALL_MESSAGE}")

set(CMAKE_INSTALL_BINARY_DIR bin CACHE STRING "Installation subdirectory for compiled binaries.")
set(CMAKE_INSTALL_SOURCE_DIR include CACHE STRING "Installation subdirectory for source code.")

# GeoViz project.
set(PROJECT_NAME GeoViz)
project(${PROJECT_NAME})

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${INSTALL_ROOT} CACHE PATH "${INSTALL_MESSAGE}")
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# Set version numbers.
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_FEATURE 0)
set(VERSION_PATCH 0)
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_FEATURE}.${VERSION_PATCH}")
message(STATUS "${PROJECT_NAME} version: ${VERSION}")

# Configure a header file to pass CMake settings to the source code.
configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/geoviz_config.h.in"
    "${PROJECT_SOURCE_DIR}/cmake/geoviz_config.h"
)
install(FILES "${PROJECT_SOURCE_DIR}/cmake/geoviz_config.h" DESTINATION "${CMAKE_INSTALL_SOURCE_DIR}/cmake")

# All source files should use include paths relative to the source root.
set(CMAKE_INCLUDE_CURRENT_DIR ON)


### LIBRARIES ###

# The GeoViz library.
add_subdirectory(geoviz)


### EXECUTABLES ###
# A few notes:
# - The executables are named after the method they expose.
# - The execuatble targets end in "_cla" for "command line application"
# - While CMake will try various file extensions when adding files, 
#   specifying the files without extension may give problems due to ambiguity.
set(CONSOLE_DIR console)
set(INSTALL_CONSOLE_DIR "${CMAKE_INSTALL_SOURCE_DIR}/${CONSOLE_DIR}")





set(TARGET_CLA_NECKLACE_MAP necklace_map_cla)
set(DIR_NECKLACE_MAP "${CONSOLE_DIR}/necklace_map")
add_executable(
    ${TARGET_CLA_NECKLACE_MAP}
    console/necklace_map/necklace_map.cpp
)
#target_link_libraries(${TARGET_CLA_NECKLACE_MAP} ${TARGET_LIB_NECKLACE_MAP})
target_link_libraries(${TARGET_CLA_NECKLACE_MAP} ${TARGET_LIB_GEOVIZ})

# Install rules.
install(TARGETS ${TARGET_CLA_NECKLACE_MAP} DESTINATION ${CMAKE_INSTALL_BINARY_DIR})
install(DIRECTORY ${DIR_NECKLACE_MAP} DESTINATION ${INSTALL_CONSOLE_DIR})










# Collect source files.

#set( INCLUDE_DIR ./test )

#file( GLOB HDRS "${INCLUDE_DIR}/*.h" )
#file( GLOB SRCS "${INCLUDE_DIR}/*.cpp" )


### MAIN ###

#set( EXEC TestCppFromWeb )


# Add the executable.
#add_executable( ${EXEC} ${HDRS} ${SRCS} )
#set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXEC} )
#set_property( TARGET ${EXEC} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$(Configuration)" )
#set_target_properties( ${EXEC} PROPERTIES DEBUG_POSTFIX d )

#if( MSVC )
    #source_group( "src" FILES ${HDRS} ${SRCS} )
    # Set the Visual Studio subsystem to console for debugging purposes.
#    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /SUBSYSTEM:CONSOLE")
#endif( MSVC )	





