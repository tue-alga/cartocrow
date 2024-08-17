set(SOURCES
    render_path_demo.cpp
)

add_executable(render_path_demo ${SOURCES})

target_link_libraries(
    render_path_demo
    PRIVATE
    ${COMMON_CLA_TARGET}
    core
    renderer
    Qt5::Widgets
)

install(TARGETS render_path_demo DESTINATION ${INSTALL_BINARY_DIR})
