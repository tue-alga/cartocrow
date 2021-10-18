# The necklace_map_cla executable exposes the Necklace Map geo-visualization method.
cartocrow_add_cla(necklace_map_gui)

target_link_libraries(
    ${TARGET}
    PRIVATE
    ${COMMON_CLA_TARGET}
    necklace_map
    core
    renderer
    CGAL::CGAL
    tinyxml2::tinyxml2
    Qt5::Widgets
    glog::glog
)
