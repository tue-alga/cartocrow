# The necklace_map_cla executable exposes the Necklace Map geo-visualization method.
geoviz_add_cla(necklace_map)

target_link_libraries(
    ${TARGET}
    ${NECKLACE_MAP_TARGET}
    CGAL::CGAL
    tinyxml2
)
