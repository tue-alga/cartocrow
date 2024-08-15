set(SOURCES
    isoline_simplification_demo.cpp
)

add_executable(isoline_simplification_demo ${SOURCES})

target_link_libraries(
    isoline_simplification_demo
    PRIVATE
    ${COMMON_CLA_TARGET}
    core
    renderer
    isoline_simplification
    CGAL::CGAL
    Qt5::Widgets
)

install(TARGETS isoline_simplification DESTINATION ${INSTALL_BINARY_DIR})
