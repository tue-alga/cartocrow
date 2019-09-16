# The GeoViz library implements several methods for algorithmic geo-visualization developed at the AGA group of Eindhoven Technical University.
set(GEOVIZ_TARGET geoviz)
set(GEOVIZ_TARGET ${GEOVIZ_TARGET} PARENT_SCOPE)
set(GEOVIZ_INSTALL_DIR "${CMAKE_INSTALL_SOURCE_DIR}/geoviz")
set(GEOVIZ_EXPORT_TARGETS GeoVizTargets)


### LIBRARIES ###

# Add the individual libraries per geo-visualization method.
# Note that each of these will have to be added to the GeoViz interface target and the GeoViz library archive.
add_subdirectory(necklace_map)
add_subdirectory(flow_diagram)


# The GeoViz library is an "interface" library:
# it forwards linking to a collection of other libraries.
# Note that this is generates a CMake construct, not a physical library.
add_library(${GEOVIZ_TARGET} INTERFACE)
target_link_libraries(
    ${GEOVIZ_TARGET} INTERFACE
    ${NECKLACE_MAP_TARGET}
    ${FLOW_DIAGRAM_TARGET}
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
        $<TARGET_FILE:${NECKLACE_MAP_TARGET}>
        $<TARGET_FILE:${FLOW_DIAGRAM_TARGET}>
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${GEOVIZ_ARCHIVE} DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR})
endif(GEOVIZ_CREATE_ARCHIVE)
