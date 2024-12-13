set(SOURCES
	core.cpp
	ipe_reader.cpp
	boundary_map.cpp
	region_arrangement.cpp
	region_map.cpp
	timer.cpp
	bezier.cpp
	rectangle_helpers.cpp
	cs_curve_helpers.cpp
	cs_polygon_helpers.cpp
	cs_polygonset_helpers.cpp
	cs_polyline_helpers.cpp
	cs_types.cpp
	vector_helpers.cpp
)
set(HEADERS
	centroid.h
	core.h
	ipe_reader.h
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
	cs_curve_helpers.h
	cs_polygon_helpers.h
	cs_polygonset_helpers.h
	cs_polyline_helpers.h
	cs_types.h
	vector_helpers.h
)

add_library(core ${SOURCES})
target_link_libraries(core
	PUBLIC Ipe::ipelib
	PRIVATE glog::glog
)

cartocrow_install_module(core)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/core)
