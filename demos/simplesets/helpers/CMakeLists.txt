set(SOURCES
    circle_convex_hull.cpp
)

add_executable(circle_convex_hull_demo ${SOURCES})

target_link_libraries(
    circle_convex_hull_demo
    PRIVATE
    ${COMMON_CLA_TARGET}
    core
    renderer
    simplesets
    CGAL::CGAL
    Qt5::Widgets
)

install(TARGETS circle_convex_hull_demo DESTINATION ${INSTALL_BINARY_DIR})
