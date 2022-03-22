cartocrow_add_cla(flow_map_gui)

target_link_libraries(
    ${TARGET}
    PRIVATE
    ${COMMON_CLA_TARGET}
    flow_map
    core
    renderer
    CGAL::CGAL
    tinyxml2::tinyxml2
    Qt5::Widgets
    glog::glog
)
