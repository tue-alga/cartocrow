# The test_necklace_map executable exposes the Necklace Map geo-visualization method unit tests.
geoviz_add_test(necklace_map)

test_link_libraries(
    ${NECKLACE_MAP_IO_TARGET}
    ${COMMON_CLA_TARGET}
    ${NECKLACE_MAP_TARGET}
    ${GEOVIZ_COMMON_TARGET}
    CGAL::CGAL
    tinyxml2
    gflags
    glog
    stdc++fs
)

#test_link_libraries(
#    ${NECKLACE_MAP_IO_TARGET}
#    ${COMMON_CLA_TARGET}
#    ${NECKLACE_MAP_TARGET}
#    CGAL::CGAL
#    tinyxml2
#)
