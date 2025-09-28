set(SOURCES
    optimization_demo.cpp
    cost_graph.cpp
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
)

install(TARGETS optimization_demo DESTINATION ${INSTALL_BINARY_DIR})
