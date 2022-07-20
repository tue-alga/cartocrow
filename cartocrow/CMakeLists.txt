set(CARTOCROW_INSTALL_DIR "${CMAKE_INSTALL_SOURCE_DIR}/cartocrow")
set(CARTOCROW_EXPORT_TARGETS CartoCrowTargets)

macro(cartocrow_add_library TARGET INSTALL_DIR)
    # - While CMake will try various file extensions when adding files,
    # specifying the files without extension may give problems due to ambiguity.
    file(GLOB_RECURSE HDRS "*.h")
    file(GLOB_RECURSE SRCS "*.cpp")

    add_library(${TARGET} ${HDRS} ${SRCS})

    # Install rules.
    install(
        TARGETS ${TARGET}
        EXPORT ${CARTOCROW_EXPORT_TARGETS}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_SOURCE_DIR}
    )
    install_files_and_directories("${HDRS}" ${INSTALL_DIR})
endmacro(cartocrow_add_library)

add_subdirectory(core)
add_subdirectory(renderer)
add_subdirectory(necklace_map)
add_subdirectory(flow_map)
add_subdirectory(visvalingam-whyatt)

add_library(cartocrow_lib INTERFACE)
target_link_libraries(
    cartocrow_lib INTERFACE
    core
    renderer
    necklace_map
    flow_map
    vw_simplification
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
