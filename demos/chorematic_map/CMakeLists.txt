set(SOURCES
    chorematic_map_demo.cpp
    parse_input.cpp
)

add_executable(chorematic_map_demo ${SOURCES})

target_link_libraries(
    chorematic_map_demo
    PRIVATE
    ${COMMON_CLA_TARGET}
    core
    chorematic_map
    renderer
    CGAL::CGAL
    Qt5::Widgets
)

install(TARGETS chorematic_map_demo DESTINATION ${INSTALL_BINARY_DIR})

add_subdirectory(performance_test)