# The test_necklace_map executable exposes the Necklace Map geo-visualization method unit tests.
cartocrow_add_test(necklace_map)

test_link_libraries(
    ${COMMON_CLA_TARGET}
    ${NECKLACE_MAP_TARGET}
    ${CARTOCROW_COMMON_TARGET}
    CGAL::CGAL
    tinyxml2::tinyxml2
    gflags
    glog::glog
)
