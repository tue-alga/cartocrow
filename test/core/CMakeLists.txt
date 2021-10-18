cartocrow_add_test(core)

test_link_libraries(
    ${COMMON_CLA_TARGET}
    core
    CGAL::CGAL
    tinyxml2::tinyxml2
    gflags
    glog::glog
)
