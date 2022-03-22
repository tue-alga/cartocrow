set(INSTALL_TEST_DIR "${CMAKE_INSTALL_SOURCE_DIR}/test")
if(MSVC)
    # Set the Visual Studio subsystem to console for debugging purposes.
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /SUBSYSTEM:CONSOLE")
endif(MSVC)


### MACROS ###

macro(add_test_exe_properties target)
    set_target_properties( ${target} PROPERTIES DEBUG_POSTFIX d )
    target_link_libraries(
        ${TARGET}
        UnitTest++
    )

    if(MSVC)
        # Add Visual Studio specific constructs to a target.
        source_group( "src" FILES ${HDRS} ${SRCS} )
        set_target_properties( ${target} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(Configuration)" )
    endif(MSVC)
endmacro(add_test_exe_properties)

macro(cartocrow_add_test NAME)
    set(TARGET "test_${NAME}")
    set(INSTALL_DIR ${INSTALL_TEST_DIR}/${NAME})

    # - While CMake will try various file extensions when adding files,
    #   specifying the files without extension may give problems due to ambiguity.
    file(GLOB_RECURSE HDRS "*.h")
    file(GLOB_RECURSE SRCS "*.cpp")

    add_executable( ${TARGET} ${HDRS} ${SRCS} )
    add_test_exe_properties(${TARGET})

    # Add a custom target to run the test target on build.
    add_custom_target(
        run_${TARGET}
        "${TARGET}"
        DEPENDS ${TARGET}
        COMMENT "Running unit test suite: ${TARGET}"
    )
endmacro(cartocrow_add_test)

macro(test_link_libraries)
    foreach(LIBRARY ${ARGN})
        target_link_libraries(
            ${TARGET}
            ${LIBRARY}
        )
        target_link_libraries(
            ${TEST_ALL_TARGET}
            ${LIBRARY}
        )
    endforeach(LIBRARY)
endmacro(test_link_libraries)


# Configure a header file to pass CMake settings to the source code.
set(TEST_DATA_DIR ${CMAKE_SOURCE_DIR}/data)
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/cartocrow_test_config.h.in"
    "${CMAKE_SOURCE_DIR}/cmake/cartocrow_test_config.h"
)
install(FILES "${CMAKE_SOURCE_DIR}/cmake/cartocrow_test_config.h" DESTINATION "${CMAKE_INSTALL_SOURCE_DIR}/cmake")


# Most command-line applications use gflags to parse command-line arguments.
find_package(UnitTest++ REQUIRED)

### LIBRARIES ###

### EXECUTABLES ###

# The test_all executable runs all unit tests
cartocrow_add_test(all)
set(TEST_ALL_TARGET ${TARGET})
target_compile_definitions(${TARGET} PUBLIC "CARTOCROW_DISABLE_SUITE_MAIN")


# A few notes:
# - The executables are named after the library they test.
# - The executable targets start with "test_".

add_subdirectory(core)
add_subdirectory(necklace_map)
#add_subdirectory(flow_map)
