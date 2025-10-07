set(SOURCES
    bezier_demo.cpp
)

add_executable(bezier_demo ${SOURCES})

target_link_libraries(
    bezier_demo
    PRIVATE
    core
    renderer
    widgets
    Qt5::Widgets
)

install(TARGETS bezier_demo DESTINATION ${INSTALL_BINARY_DIR})
