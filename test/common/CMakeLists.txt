# The test_common executable exposes the common geo-visualization method unit tests.
geoviz_add_test(common)

test_link_libraries(
    ${COMMON_CLA_TARGET}
    ${GEOVIZ_COMMON_TARGET}
    CGAL::CGAL
    tinyxml2::tinyxml2
    gflags
    glog::glog
)