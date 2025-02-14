set(SOURCES
   vw_demo.cpp
)
add_executable(vw_demo ${SOURCES})
target_link_libraries(
    vw_demo
    PRIVATE
    ${COMMON_CLA_TARGET}
    core
    simplification
    renderer
    CGAL::CGAL
    Qt5::Widgets
    glog::glog
)
install(TARGETS vw_demo DESTINATION ${INSTALL_BINARY_DIR})

set(SOURCES
   ksbb_demo.cpp
)
add_executable(ksbb_demo ${SOURCES})
target_link_libraries(
    ksbb_demo
    PRIVATE
    ${COMMON_CLA_TARGET}
    core
    simplification
    renderer
    CGAL::CGAL
    Qt5::Widgets
    glog::glog
)
install(TARGETS ksbb_demo DESTINATION ${INSTALL_BINARY_DIR})