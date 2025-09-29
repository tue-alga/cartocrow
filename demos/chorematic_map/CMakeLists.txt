set(SOURCES
    chorematic_map_demo.cpp
)

add_executable(chorematic_map_demo ${SOURCES})

target_link_libraries(
    chorematic_map_demo
    PRIVATE
    ${COMMON_CLA_TARGET}
    core
    chorematic_map
    renderer
    reader
    CGAL::CGAL
    GDAL::GDAL
    widgets
    Qt5::Widgets
)

install(TARGETS chorematic_map_demo DESTINATION ${INSTALL_BINARY_DIR})
