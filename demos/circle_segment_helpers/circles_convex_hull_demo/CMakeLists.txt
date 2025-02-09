set(SOURCES
    circles_convex_hull_demo.cpp
)

add_executable(circles_convex_hull_demo ${SOURCES})

target_link_libraries(
    circles_convex_hull_demo
    PRIVATE
    ${COMMON_CLA_TARGET}
    core
    renderer
    circle_segment_helpers
    CGAL::CGAL
    Qt5::Widgets
)

install(TARGETS circles_convex_hull_demo DESTINATION ${INSTALL_BINARY_DIR})
