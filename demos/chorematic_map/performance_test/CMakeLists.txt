set(SOURCES
    performance_test.cpp
)

add_executable(chorematic_performance_test ${SOURCES})

target_link_libraries(
    chorematic_performance_test
    PRIVATE
    ${COMMON_CLA_TARGET}
    core
    chorematic_map
    renderer
    CGAL::CGAL
    GDAL::GDAL
    Qt5::Widgets
)

install(TARGETS chorematic_performance_test DESTINATION ${INSTALL_BINARY_DIR})
