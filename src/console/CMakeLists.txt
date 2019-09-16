set(INSTALL_CONSOLE_DIR "${CMAKE_INSTALL_SOURCE_DIR}/console")
if(MSVC)
    # Set the Visual Studio subsystem to console for debugging purposes.
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /SUBSYSTEM:CONSOLE")
endif(MSVC)

# Macro for adding Visual Studio specific constructs to a target.
macro(add_exe_properties target)
    set_target_properties( ${target} PROPERTIES DEBUG_POSTFIX d )

    if(MSVC)
        source_group( "src" FILES ${HDRS} ${SRCS} )
        set_target_properties( ${target} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(Configuration)" )
    endif(MSVC)
endmacro(add_exe_properties)

### EXECUTABLES ###
# A few notes:
# - The executables are named after the method they expose.
# - The execuatble targets end in "_cla" for "command line application"

add_subdirectory(necklace_map)

# Set the default executable to run in Visual Studio.
if(MSVC)
    set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${NECKLACE_MAP_CLA_TARGET} )
endif(MSVC)
