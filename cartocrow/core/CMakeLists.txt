set(SOURCES
	core.cpp
	region_arrangement.cpp
	timer.cpp
	bezier.cpp
	rectangle_helpers.cpp
	vector_helpers.cpp
	transform_helpers.cpp
)
set(HEADERS
	centroid.h
	core.h
	boundary_map.h
	arrangement_map.h
	region_arrangement.h
	region_map.h
	timer.h
	bezier.h
	rectangle_helpers.h
	arrangement_helpers.h
	polyline.h
	general_polyline.h
	vector_helpers.h
	halfplane.h
	transform_helpers.h
	cropped_voronoi.h
)

add_library(core ${SOURCES})

cartocrow_install_module(core)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/core)
