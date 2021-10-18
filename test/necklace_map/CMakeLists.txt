# The test_necklace_map executable exposes the Necklace Map geo-visualization method unit tests.
cartocrow_add_test(necklace_map)

test_link_libraries(
    ${COMMON_CLA_TARGET}
    necklace_map
    core
    CGAL::CGAL
    tinyxml2::tinyxml2
    gflags
    glog::glog
)
