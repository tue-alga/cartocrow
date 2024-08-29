set(SOURCES
    renderer_demo.cpp
)

add_executable(renderer_demo ${SOURCES})

target_link_libraries(
    renderer_demo
    PRIVATE
    ${COMMON_CLA_TARGET}
    core
    renderer
    Qt5::Widgets
)

install(TARGETS renderer_demo DESTINATION ${INSTALL_BINARY_DIR})
