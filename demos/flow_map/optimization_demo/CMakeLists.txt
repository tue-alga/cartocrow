set(SOURCES
    optimization_demo.cpp
)

add_executable(optimization_demo ${SOURCES})

target_link_libraries(
    optimization_demo
    PRIVATE
    ${COMMON_CLA_TARGET}
    core
    flow_map
    renderer
    CGAL::CGAL
    Qt5::Widgets
    glog::glog
)

install(TARGETS optimization_demo DESTINATION ${INSTALL_BINARY_DIR})
