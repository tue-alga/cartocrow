set(SOURCES
    circle_tangents_demo.cpp
)

add_executable(circle_tangents_demo ${SOURCES})

target_link_libraries(
        circle_tangents_demo
        PRIVATE
        ${COMMON_CLA_TARGET}
        core
        renderer
        circle_segment_helpers
        Qt5::Widgets
)

install(TARGETS circle_tangents_demo DESTINATION ${INSTALL_BINARY_DIR})
