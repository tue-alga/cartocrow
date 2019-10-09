# The print_arguments_cla executable prints its arguments formatted as a div HTML element.
set(PRINT_ARGUMENTS_CLA_TARGET "print_arguments_cla")
set(PRINT_ARGUMENTS_CLA_PHP "run_print_arguments.php")

# - While CMake will try various file extensions when adding files,
#   specifying the files without extension may give problems due to ambiguity.
file(GLOB_RECURSE HDRS "*.h")
file(GLOB_RECURSE SRCS "*.cpp")

add_executable( ${PRINT_ARGUMENTS_CLA_TARGET} ${HDRS} ${SRCS} )
add_exe_properties(${PRINT_ARGUMENTS_CLA_TARGET})

target_link_libraries(
    ${PRINT_ARGUMENTS_CLA_TARGET}
    gflags
)

# Install rules.
set(INSTALL_PRINT_ARGUMENTS_CLA_DIR ${INSTALL_CONSOLE_DIR}/print_arguments)
install(TARGETS ${PRINT_ARGUMENTS_CLA_TARGET} DESTINATION ${CMAKE_INSTALL_BINARY_DIR})
install_files_and_directories("${HDRS}" ${INSTALL_PRINT_ARGUMENTS_CLA_DIR})
install_files_and_directories("${SRCS}" ${INSTALL_PRINT_ARGUMENTS_CLA_DIR})

# PHP script to run the executable.
# Note that the configure file uses the ${CURRENT_CLA} variable to determine the executable to run.
set(CURRENT_CLA ${PRINT_ARGUMENTS_CLA_TARGET})
configure_file(
    "${SOURCE_CONFIG_DIR}/run_cla.php.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PRINT_ARGUMENTS_CLA_PHP}"
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PRINT_ARGUMENTS_CLA_PHP}" DESTINATION "${CMAKE_INSTALL_SCRIPT_DIR}")
