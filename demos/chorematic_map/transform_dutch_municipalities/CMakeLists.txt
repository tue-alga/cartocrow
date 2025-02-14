set(SOURCES
    transform_dutch_municipalities.cpp
)

add_executable(transform_dutch_municipalities ${SOURCES})

target_link_libraries(
    transform_dutch_municipalities
    PRIVATE
    ${COMMON_CLA_TARGET}
    core
    chorematic_map
    simplification
    renderer
    GDAL::GDAL
    CGAL::CGAL
    Qt5::Widgets
)

install(TARGETS transform_dutch_municipalities DESTINATION ${INSTALL_BINARY_DIR})
