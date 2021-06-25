# The necklace_map_cla executable exposes the Necklace Map geo-visualization method.
geoviz_add_cla(necklace_map)

target_link_libraries(
    ${TARGET}
    ${COMMON_CLA_TARGET}
    ${NECKLACE_MAP_TARGET}
    ${GEOVIZ_COMMON_TARGET}
    CGAL::CGAL
    tinyxml2::tinyxml2
)

# PHP script to run the executable.
# Note that the configure file uses the ${CURRENT_CLA} variable to determine the executable to run.
set(PHP "run_necklace_map.php")
set(CURRENT_CLA ${TARGET})
configure_file(
    "run_necklace_map.php.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PHP}"
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PHP}" DESTINATION "${INSTALL_SCRIPT_DIR}")
