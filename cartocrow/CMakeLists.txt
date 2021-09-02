# The CartoCrow library implements several methods for algorithmic geo-visualization developed at the AGA group of Eindhoven Technical University.
set(CARTOCROW_TARGET cartocrow)
set(CARTOCROW_TARGET ${CARTOCROW_TARGET} PARENT_SCOPE)
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
    install_files_and_directories("${SRCS}" ${INSTALL_DIR})
endmacro(cartocrow_add_library)


### LIBRARIES ###

# Add the individual libraries per geo-visualization method.
# Note that each of these will have to be added to the CartoCrow interface target and the CartoCrow library archive.
add_subdirectory(common)
add_subdirectory(renderer)
add_subdirectory(necklace_map)
add_subdirectory(flow_map)

set(CARTOCROW_COMMON_TARGET ${CARTOCROW_COMMON_TARGET} PARENT_SCOPE)
set(CARTOCROW_RENDERER_TARGET ${CARTOCROW_RENDERER_TARGET} PARENT_SCOPE)
set(NECKLACE_MAP_TARGET ${NECKLACE_MAP_TARGET} PARENT_SCOPE)
set(FLOW_MAP_TARGET ${FLOW_MAP_TARGET} PARENT_SCOPE)


# The CartoCrow library is an "interface" library:
# it forwards linking to a collection of other libraries.
# Note that this is generates a CMake construct, not a physical library.
add_library(${CARTOCROW_TARGET} INTERFACE)
target_link_libraries(
    ${CARTOCROW_TARGET} INTERFACE
    ${CARTOCROW_COMMON_TARGET}
    ${CARTOCROW_RENDERER_TARGET}
    ${NECKLACE_MAP_TARGET}
    ${FLOW_MAP_TARGET}
)

install(
    TARGETS ${CARTOCROW_TARGET}
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
        $<TARGET_FILE:${CARTOCROW_COMMON_TARGET}>
        $<TARGET_FILE:${CARTOCROW_RENDERER_TARGET}>
        $<TARGET_FILE:${NECKLACE_MAP_TARGET}>
        $<TARGET_FILE:${FLOW_MAP_TARGET}>
    )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${CARTOCROW_ARCHIVE} DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR})
endif(CARTOCROW_CREATE_ARCHIVE)

