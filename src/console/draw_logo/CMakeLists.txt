# The draw_logo_cla executable prints a logo formatted as a svg HTML element.
set(DRAW_LOGO_CLA_TARGET "draw_logo_cla")
set(DRAW_LOGO_CLA_PHP "run_draw_logo.php")

# - While CMake will try various file extensions when adding files,
#   specifying the files without extension may give problems due to ambiguity.
file(GLOB_RECURSE HDRS "*.h")
file(GLOB_RECURSE SRCS "*.cpp")

add_executable( ${DRAW_LOGO_CLA_TARGET} ${HDRS} ${SRCS} )
add_exe_properties(${DRAW_LOGO_CLA_TARGET})

target_link_libraries(
    ${DRAW_LOGO_CLA_TARGET}
    gflags
)

# Install rules.
set(INSTALL_DRAW_LOGO_CLA_DIR ${INSTALL_CONSOLE_DIR}/draw_logo)
install(TARGETS ${DRAW_LOGO_CLA_TARGET} DESTINATION ${CMAKE_INSTALL_BINARY_DIR})
install_files_and_directories("${HDRS}" ${INSTALL_DRAW_LOGO_CLA_DIR})
install_files_and_directories("${SRCS}" ${INSTALL_DRAW_LOGO_CLA_DIR})

# PHP script to run the executable.
# Note that the configure file uses the ${CURRENT_CLA} variable to determine the executable to run.
set(CURRENT_CLA ${DRAW_LOGO_CLA_TARGET})
configure_file(
    "${SOURCE_CONFIG_DIR}/run_cla.php.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${DRAW_LOGO_CLA_PHP}"
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${DRAW_LOGO_CLA_PHP}" DESTINATION "${CMAKE_INSTALL_SCRIPT_DIR}")
