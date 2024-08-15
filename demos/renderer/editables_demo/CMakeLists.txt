set(SOURCES
    editables_demo.cpp
)

add_executable(editables_demo ${SOURCES})

target_link_libraries(
    editables_demo
    PRIVATE
    ${COMMON_CLA_TARGET}
    core
    renderer
    Qt5::Widgets
)

install(TARGETS editables_demo DESTINATION ${INSTALL_BINARY_DIR})
