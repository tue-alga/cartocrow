# The flow_map_cla executable exposes the Flow Map geo-visualization method.
cartocrow_add_cla(flow_map)

target_link_libraries(
    ${TARGET}
    PRIVATE
    ${COMMON_CLA_TARGET}
    ${FLOW_MAP_TARGET}
    ${CARTOCROW_COMMON_TARGET}
    CGAL::CGAL
    tinyxml2::tinyxml2
)

# PHP script to run the executable.
# Note that the configure file uses the ${CURRENT_CLA} variable to determine the executable to run.
set(PHP "run_flow_map.php")
set(CURRENT_CLA ${TARGET})
configure_file(
    "run_flow_map.php.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PHP}"
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PHP}" DESTINATION "${INSTALL_SCRIPT_DIR}")
