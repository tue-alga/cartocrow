set(SOURCES
    spiral_tree_demo.cpp
)

add_executable(spiral_tree_demo ${SOURCES})

target_link_libraries(
    spiral_tree_demo
    PRIVATE
    ${COMMON_CLA_TARGET}
    core
    flow_map
    renderer
    CGAL::CGAL
    Qt5::Widgets
)

install(TARGETS spiral_tree_demo DESTINATION ${INSTALL_BINARY_DIR})
