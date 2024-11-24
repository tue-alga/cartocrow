set(SOURCES
    cs_polyline_helpers_demo.cpp
)

add_executable(cs_polyline_helpers_demo ${SOURCES})

target_link_libraries(
    cs_polyline_helpers_demo
    PRIVATE
    ${COMMON_CLA_TARGET}
    core
    renderer
    simplesets
    CGAL::CGAL
    Qt5::Widgets
)

install(TARGETS cs_polyline_helpers_demo DESTINATION ${INSTALL_BINARY_DIR})
