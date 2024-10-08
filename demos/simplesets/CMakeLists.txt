set(SOURCES
    simplesets_demo.cpp
    colors.cpp
)

add_executable(simplesets_demo ${SOURCES})

target_link_libraries(
    simplesets_demo
    PRIVATE
    ${COMMON_CLA_TARGET}
    core
    renderer
    simplesets
    CGAL::CGAL
    Qt5::Widgets
)

install(TARGETS simplesets_demo DESTINATION ${INSTALL_BINARY_DIR})

add_subdirectory(helpers)