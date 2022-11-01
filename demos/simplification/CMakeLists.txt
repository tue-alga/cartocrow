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