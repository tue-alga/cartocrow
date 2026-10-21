set(CARTOCROW_INSTALL_DIR "${CMAKE_INSTALL_SOURCE_DIR}/cartocrow")
set(CARTOCROW_EXPORT_TARGETS CartoCrowTargets)

macro(cartocrow_install_module NAME)
    install(
        TARGETS ${NAME}
        EXPORT ${CARTOCROW_EXPORT_TARGETS}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_SOURCE_DIR}
    )
endmacro(cartocrow_install_module)

add_subdirectory(core)
add_subdirectory(circle_segment_helpers)
add_subdirectory(data_structures)
add_subdirectory(renderer)
add_subdirectory(reader)
add_subdirectory(utils)
if (NOT EMSCRIPTEN)
    add_subdirectory(widgets)
endif()

add_library(cartocrow_lib INTERFACE)
if (NOT EMSCRIPTEN)
target_link_libraries(
    cartocrow_lib INTERFACE
    core
    circle_segment_helpers
    data_structures
    renderer
    reader
    utils
    widgets
)
else()
target_link_libraries(
    cartocrow_lib INTERFACE
    core
    data_structures
    circle_segment_helpers
    renderer
    reader
    utils
)
endif()

cartocrow_install_module(cartocrow_lib)
install(
    EXPORT ${CARTOCROW_EXPORT_TARGETS}
    FILE "${CARTOCROW_EXPORT_TARGETS}.cmake"
    NAMESPACE cartocrow::
    DESTINATION ${INSTALL_CONFIG_DIR}/CartoCrow
)
