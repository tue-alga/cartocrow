# The map_regions_cla executable analyses a map SVG file and returns if it is valid.
cartocrow_add_cla(map_regions)

target_link_libraries(
    ${TARGET}
    PRIVATE
    ${NECKLACE_MAP_IO_TARGET}
    ${COMMON_CLA_TARGET}
    ${NECKLACE_MAP_TARGET}
    ${CARTOCROW_COMMON_TARGET}
    CGAL::CGAL
    tinyxml2::tinyxml2
)

# PHP script to run the executable.
# Note that the configure file uses the ${CURRENT_CLA} variable to determine the executable to run.
set(PHP "run_map_regions.php")
set(CURRENT_CLA ${TARGET})
configure_file(
    "run_map_regions.php.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PHP}"
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PHP}" DESTINATION "${INSTALL_SCRIPT_DIR}")
