set(INSTALL_CONSOLE_DIR "${CMAKE_INSTALL_SOURCE_DIR}/console")
if(MSVC)
    # Set the Visual Studio subsystem to console for debugging purposes.
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /SUBSYSTEM:CONSOLE")
endif(MSVC)


### MACROS ###

macro(cartocrow_add_console_library TARGET INSTALL_DIR)
    # - While CMake will try various file extensions when adding files,
    #   specifying the files without extension may give problems due to ambiguity.
    file(GLOB_RECURSE HDRS "*.h")
    file(GLOB_RECURSE SRCS "*.cpp")

    add_library(${TARGET} ${HDRS} ${SRCS})

    # Install rules.
    install(
        TARGETS ${TARGET}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    )
endmacro(cartocrow_add_console_library)

macro(add_cla_exe_properties target)
    set_target_properties( ${target} PROPERTIES DEBUG_POSTFIX d )
    target_link_libraries(
        ${target}
        PRIVATE
        ${COMMON_CLA_TARGET}
        gflags
        glog::glog
    )

    if(MSVC)
        # Add Visual Studio specific constructs to a target.
        source_group( "src" FILES ${HDRS} ${SRCS} )
        set_target_properties( ${target} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(Configuration)" )
    endif(MSVC)
endmacro(add_cla_exe_properties)

macro(cartocrow_add_cla NAME)
    set(TARGET "${NAME}")
    set(INSTALL_DIR ${INSTALL_CONSOLE_DIR}/${NAME})

    # - While CMake will try various file extensions when adding files,
    #   specifying the files without extension may give problems due to ambiguity.
    file(GLOB_RECURSE HDRS "*.h")
    file(GLOB_RECURSE SRCS "*.cpp")

    add_executable( ${TARGET} ${HDRS} ${SRCS} )
    add_cla_exe_properties(${TARGET})

    # Install rules.
    install(TARGETS ${TARGET} DESTINATION ${INSTALL_BINARY_DIR})

    # PHP script to run the executable.
    # Note that the configure file uses the ${CURRENT_CLA} variable to determine the executable to run.
    #set(CURRENT_CLA ${TARGET})
    #configure_file(
    #    "${CMAKE_MODULE_PATH}/run_cla.php.in"
    #    "${CMAKE_CURRENT_BINARY_DIR}/${PHP}"
    #)
    #install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PHP}" DESTINATION "${INSTALL_SCRIPT_DIR}")
endmacro(cartocrow_add_cla)


# Most command-line applications use gflags to parse command-line arguments.
find_package(gflags REQUIRED)

### LIBRARIES ###

add_subdirectory(common)

# Propagate variables up.
set(COMMON_CLA_TARGET ${COMMON_CLA_TARGET} PARENT_SCOPE)

### EXECUTABLES ###
add_subdirectory(print_arguments)
add_subdirectory(draw_logo)
add_subdirectory(multi_library_app)
add_subdirectory(necklace_map_cla)
add_subdirectory(necklace_map_gui)
add_subdirectory(map_regions)
#add_subdirectory(flow_map_cla)
add_subdirectory(flow_map_gui)
add_subdirectory(visvalingam-whyatt_cla)

# Set the default executable to run in Visual Studio.
if(MSVC)
    set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${NECKLACE_MAP_CLA_TARGET} )
endif(MSVC)
