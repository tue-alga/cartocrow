set(SOURCES
    approximate_convex_hull_of_disks.cpp
    cavc_helpers.cpp
    circle_tangents.cpp
    cs_curve_helpers.cpp
    cs_polygon_helpers.cpp
    cs_polyline_helpers.cpp
    cs_render_helpers.cpp
    cs_types.cpp
    poly_line_gon_intersection.cpp
)

set(HEADERS
    approximate_convex_hull_of_disks.h
    cavc_helpers.h
    circle_tangents.h
    cs_curve_helpers.h
    cs_polygon_helpers.h
    cs_polyline_helpers.h
    cs_render_helpers.h
    cs_types.h
    poly_line_gon_intersection.h
)

add_library(circle_segment_helpers ${SOURCES})
target_link_libraries(circle_segment_helpers
    PUBLIC core
           renderer
)

cartocrow_install_module(circle_segment_helpers)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/circle_segment_helpers)
