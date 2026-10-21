set(SOURCES
	core.cpp
	cubic_bezier.cpp
	ellipse.cpp
	geometric_feature.cpp
	multi_point.cpp
	multipolygon_with_holes.cpp
	rectangle_helpers.cpp
	region_arrangement.cpp
	segment_delaunay_graph_helpers.cpp
	transform_helpers.cpp
	vector_helpers.cpp
)
set(HEADERS
	arrangement_helpers.h
	arrangement_map.h
	arrangement_map.hpp
	boundary_map.h
	core.h
	cropped_voronoi.h
	cubic_bezier.h
	delaunay_voronoi_helpers.h
	ellipse.h
	general_polyline.h
	geometric_feature.h
	halfplane.h
	polygon_helpers.h
	polyline.h
	multi_point.h
	multipolygon_with_holes.h
	multi_polyline.h
	rectangle_helpers.h
	region_arrangement.h
	region_map.h
	root_finding_helpers.h
	segment_delaunay_graph_helpers.h
	segment_delaunay_graph_helpers_cgal_adapted.h
	straight_geometry.h
	straight_geometry_collection.h
	transform_helpers.h
	vector_helpers.h
)

add_library(core ${SOURCES})

cartocrow_install_module(core)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/core)
