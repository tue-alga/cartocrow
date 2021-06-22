# The GeoViz library implements several methods for algorithmic geo-visualization developed at the AGA group of Eindhoven Technical University.
set(GEOVIZ_TARGET geoviz)
set(GEOVIZ_TARGET ${GEOVIZ_TARGET} PARENT_SCOPE)
set(GEOVIZ_INSTALL_DIR "${CMAKE_INSTALL_SOURCE_DIR}/geoviz")
set(GEOVIZ_EXPORT_TARGETS GeoVizTargets)


### MACROS ###

macro(geoviz_add_library TARGET INSTALL_DIR)
    # - While CMake will try various file extensions when adding files,
    #   specifying the files without extension may give problems due to ambiguity.
    file(GLOB_RECURSE HDRS "*.h")
    file(GLOB_RECURSE SRCS "*.cpp")

    add_library(${TARGET} ${HDRS} ${SRCS})

    # Install rules.
    install(
        TARGETS ${TARGET}
        EXPORT ${GEOVIZ_EXPORT_TARGETS}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    )
    install_files_and_directories("${HDRS}" ${INSTALL_DIR})
    install_files_and_directories("${SRCS}" ${INSTALL_DIR})
endmacro(geoviz_add_library)


### LIBRARIES ###

# Add the individual libraries per geo-visualization method.
# Note that each of these will have to be added to the GeoViz interface target and the GeoViz library archive.
add_subdirectory(common)
add_subdirectory(necklace_map)
add_subdirectory(flow_map)

set(GEOVIZ_COMMON_TARGET ${GEOVIZ_COMMON_TARGET} PARENT_SCOPE)
set(NECKLACE_MAP_TARGET ${NECKLACE_MAP_TARGET} PARENT_SCOPE)
set(FLOW_MAP_TARGET ${FLOW_MAP_TARGET} PARENT_SCOPE)


# The GeoViz library is an "interface" library:
# it forwards linking to a collection of other libraries.
# Note that this is generates a CMake construct, not a physical library.
add_library(${GEOVIZ_TARGET} INTERFACE)
target_link_libraries(
    ${GEOVIZ_TARGET} INTERFACE
    ${GEOVIZ_COMMON_TARGET}
    ${NECKLACE_MAP_TARGET}
    ${FLOW_MAP_TARGET}
)

install(
    TARGETS ${GEOVIZ_TARGET}
    EXPORT ${GEOVIZ_EXPORT_TARGETS}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
)
install(
    EXPORT ${GEOVIZ_EXPORT_TARGETS}
    FILE "${GEOVIZ_EXPORT_TARGETS}.cmake"
    NAMESPACE GeoViz::
    DESTINATION ${INSTALL_CONFIG_DIR}/GeoViz
)

if(GEOVIZ_CREATE_ARCHIVE)
    # We also create an archive target: a physical library containing the others.
    # note that this target should not be linked within this project: use the interface target instead.
    set(GEOVIZ_ARCHIVE ${CMAKE_STATIC_LIBRARY_PREFIX}geoviz${CMAKE_STATIC_LIBRARY_SUFFIX})
    add_custom_target(
        archive_geoviz ALL
        COMMAND ${CMAKE_AR} rc ${GEOVIZ_ARCHIVE}
        $<TARGET_FILE:${GEOVIZ_COMMON_TARGET}>
        $<TARGET_FILE:${NECKLACE_MAP_TARGET}>
        $<TARGET_FILE:${FLOW_MAP_TARGET}>
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${GEOVIZ_ARCHIVE} DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR})
endif(GEOVIZ_CREATE_ARCHIVE)
