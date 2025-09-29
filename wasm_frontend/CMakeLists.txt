set(SOURCES
    cartocrow.cpp
)

add_executable(cartocrow ${SOURCES})

target_link_libraries(
    cartocrow
    PRIVATE
    core
    circle_segment_helpers
    renderer
    simplesets
)
