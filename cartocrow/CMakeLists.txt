set(CARTOCROW_INSTALL_DIR "${CMAKE_INSTALL_SOURCE_DIR}/cartocrow")
set(CARTOCROW_EXPORT_TARGETS CartoCrowTargets)


### MACROS ###

macro(cartocrow_add_library TARGET INSTALL_DIR)
    # - While CMake will try various file extensions when adding files,
    #   specifying the files without extension may give problems due to ambiguity.
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


### LIBRARIES ###

# Add the individual libraries per geo-visualization method.
# Note that each of these will have to be added to the CartoCrow interface target and the CartoCrow library archive.
add_subdirectory(core)
add_subdirectory(renderer)
add_subdirectory(necklace_map)
add_subdirectory(flow_map)
add_subdirectory(visvalingam-whyatt)

# The CartoCrow library is an "interface" library:
# it forwards linking to a collection of other libraries.
# Note that this is generates a CMake construct, not a physical library.
add_library(cartocrow INTERFACE)
target_link_libraries(
    cartocrow INTERFACE
    core
    renderer
    necklace_map
    flow_map
    vw_simplification
)

install(
    TARGETS cartocrow
    EXPORT ${CARTOCROW_EXPORT_TARGETS}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_SOURCE_DIR}
)
install(
    EXPORT ${CARTOCROW_EXPORT_TARGETS}
    FILE "${CARTOCROW_EXPORT_TARGETS}.cmake"
    NAMESPACE CartoCrow::
    DESTINATION ${INSTALL_CONFIG_DIR}/CartoCrow
)

if(CARTOCROW_CREATE_ARCHIVE)
    # We also create an archive target: a physical library containing the others.
    # note that this target should not be linked within this project: use the interface target instead.
    set(CARTOCROW_ARCHIVE ${CMAKE_STATIC_LIBRARY_PREFIX}cartocrow${CMAKE_STATIC_LIBRARY_SUFFIX})
    add_custom_target(
        archive_cartocrow ALL
        COMMAND ${CMAKE_AR} rc ${CARTOCROW_ARCHIVE}
        $<TARGET_FILE:core>
        $<TARGET_FILE:renderer>
        $<TARGET_FILE:necklace_map>
        $<TARGET_FILE:flow_map>
        $<TARGET_FILE:vw_simplification>
    )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${CARTOCROW_ARCHIVE} DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR})
endif(CARTOCROW_CREATE_ARCHIVE)

