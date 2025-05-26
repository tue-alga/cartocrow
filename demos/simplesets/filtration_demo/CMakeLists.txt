set(SOURCES
    filtration_demo.cpp
    ../colors/colors.cpp
)

add_executable(filtration_demo ${SOURCES})

target_link_libraries(
    filtration_demo
    PRIVATE
    ${COMMON_CLA_TARGET}
    core
    renderer
    simplesets
    CGAL::CGAL
    widgets
    Qt5::Widgets
)

install(TARGETS filtration_demo DESTINATION ${INSTALL_BINARY_DIR})
