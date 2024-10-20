set(SOURCES
    offset_demo.cpp
    ../colors/colors.cpp
)

add_executable(offset_demo ${SOURCES})

add_subdirectory(cavc)

target_link_libraries(
    offset_demo
    PRIVATE
    ${COMMON_CLA_TARGET}
    core
    renderer
    simplesets
    CGAL::CGAL
    Qt5::Widgets
)

install(TARGETS offset_demo DESTINATION ${INSTALL_BINARY_DIR})
