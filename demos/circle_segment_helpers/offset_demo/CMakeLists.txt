set(SOURCES
    offset_demo.cpp
)

add_executable(offset_demo ${SOURCES})

target_link_libraries(
    offset_demo
    PRIVATE
    ${COMMON_CLA_TARGET}
    core
    renderer
    circle_segment_helpers
    CGAL::CGAL
    widgets
    Qt5::Widgets
)

install(TARGETS offset_demo DESTINATION ${INSTALL_BINARY_DIR})
