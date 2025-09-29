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
add_subdirectory(renderer)
add_subdirectory(simplesets)
add_subdirectory(necklace_map)
add_subdirectory(flow_map)
if (NOT EMSCRIPTEN)
    add_subdirectory(chorematic_map)
    add_subdirectory(isoline_simplification)
    add_subdirectory(reader)
    add_subdirectory(simplification)
endif()

add_library(cartocrow_lib INTERFACE)
target_link_libraries(
    cartocrow_lib INTERFACE
    core
    circle_segment_helpers
    renderer
)

install(
    TARGETS cartocrow_lib
    EXPORT ${CARTOCROW_EXPORT_TARGETS}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_SOURCE_DIR}
)
install(
    EXPORT ${CARTOCROW_EXPORT_TARGETS}
    FILE "${CARTOCROW_EXPORT_TARGETS}.cmake"
    NAMESPACE cartocrow::
    DESTINATION ${INSTALL_CONFIG_DIR}/CartoCrow
)
