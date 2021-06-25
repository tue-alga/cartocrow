# The test_flow_map executable exposes the Flow Map geo-visualization method unit tests.
geoviz_add_test(flow_map)

test_link_libraries(
    ${COMMON_CLA_TARGET}
    ${FLOW_MAP_TARGET}
    ${GEOVIZ_COMMON_TARGET}
    CGAL::CGAL
    tinyxml2::tinyxml2
    gflags
    glog::glog
)