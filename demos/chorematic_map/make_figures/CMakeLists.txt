set(SOURCES
    make_figures.cpp
)

add_executable(chorematic_make_figures ${SOURCES})

target_link_libraries(
    chorematic_make_figures
    PRIVATE
    ${COMMON_CLA_TARGET}
    core
    chorematic_map
    renderer
    CGAL::CGAL
    GDAL::GDAL
    Qt5::Widgets
)

install(TARGETS chorematic_make_figures DESTINATION ${INSTALL_BINARY_DIR})
