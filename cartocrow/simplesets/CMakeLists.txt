set(SOURCES
	types.cpp
	parse_input.cpp
    patterns/pattern.cpp
	patterns/single_point.cpp
	patterns/matching.cpp
    patterns/island.cpp
    patterns/bank.cpp
	dilated/dilated_poly.cpp
	helpers/cs_polygon_helpers.cpp
	partition_algorithm.cpp
	partition_painting.cpp
	drawing_algorithm.cpp
	grow_circles.cpp
	poly_line_gon_intersection.cpp
)
set(HEADERS
	types.h
	settings.h
	cat_point.h
	parse_input.h
	patterns/pattern.h
	patterns/poly_pattern.h
	patterns/single_point.h
	patterns/matching.h
	patterns/island.h
	patterns/bank.h
	dilated/dilated_poly.h
	helpers/point_voronoi_helpers.h
	helpers/cropped_voronoi.h
	helpers/cs_polygon_helpers.h
	partition_algorithm.h
	partition_painting.h
	partition.h
	drawing_algorithm.h
	general_polyline.h
	grow_circles.h
	poly_line_gon_intersection.h
	helpers/arrangement_helpers.h
)

add_library(simplesets ${SOURCES})
target_link_libraries(simplesets
	PUBLIC core
)

cartocrow_install_module(simplesets)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/simplesets)
