# The GeoViz library implements several methods for algorithmic geo-visualization developed at the AGA group of Eindhoven Technical University.
set(TARGET_LIB_GEOVIZ geoviz)
set(TARGET_LIB_GEOVIZ ${TARGET_LIB_GEOVIZ} PARENT_SCOPE)
set(INSTALL_GEOVIZ_DIR "${CMAKE_INSTALL_SOURCE_DIR}/geoviz")

### LIBRARIES ###

# Add the individual libraries per geo-visualization method.
add_subdirectory(necklace_map)
add_subdirectory(flow_diagram)


# The GeoViz library is an "interface" library:
# it forwards linking to a collection of other libraries.
# Note that this is generates CMake construct, not a physical library.
add_library(${TARGET_LIB_GEOVIZ} INTERFACE)
target_link_libraries(
    ${TARGET_LIB_GEOVIZ} INTERFACE
    ${TARGET_LIB_NECKLACE_MAP}
    ${TARGET_LIB_FLOW_DIAGRAM}
)

# We also create an archive target: a physical library containing the others.
# note that this target should not be linked against within this project.
set(OUTPUT_LIB_GEOVIZ ${CMAKE_STATIC_LIBRARY_PREFIX}geoviz${CMAKE_STATIC_LIBRARY_SUFFIX})
add_custom_target(
    geoviz_archive ALL
    COMMAND ${CMAKE_AR} rc ${OUTPUT_LIB_GEOVIZ}
    $<TARGET_FILE:${TARGET_LIB_NECKLACE_MAP}>
    $<TARGET_FILE:${TARGET_LIB_FLOW_DIAGRAM}>
)