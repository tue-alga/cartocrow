# The necklace_map_cla executable exposes the Necklace Map geo-visualization method.
geoviz_add_cla(necklace_map)

target_link_libraries(
    ${TARGET}
    ${NECKLACE_MAP_IO_TARGET}
    ${COMMON_CLA_TARGET}
    ${NECKLACE_MAP_TARGET}
    ${GEOVIZ_COMMON_TARGET}
    CGAL::CGAL
    tinyxml2
)
