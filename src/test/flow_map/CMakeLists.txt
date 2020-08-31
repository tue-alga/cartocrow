# The test_flow_map executable exposes the Flow Map geo-visualization method unit tests.
geoviz_add_test(flow_map)

test_link_libraries(
    ${FLOW_MAP_TARGET}
    CGAL::CGAL
)
