# The draw_logo_cla executable prints a logo formatted as an svg HTML element.
cartocrow_add_cla(draw_logo)

# PHP script to run the executable.
# Note that the configure file uses the ${CURRENT_CLA} variable to determine the executable to run.
set(PHP "run_draw_logo.php")
set(CURRENT_CLA ${TARGET})
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/run_cla.php.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PHP}"
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PHP}" DESTINATION "${INSTALL_SCRIPT_DIR}")
