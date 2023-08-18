set(SOURCES
	intersections.cpp
	node.cpp
	parameters.cpp
	painting.cpp
	place.cpp
	polar_line.cpp
	polar_point.cpp
	polar_segment.cpp
	reachable_region_algorithm.cpp
	spiral.cpp
	spiral_segment.cpp
	spiral_tree.cpp
	spiral_tree_obstructed_algorithm.cpp
	spiral_tree_unobstructed_algorithm.cpp
	sweep_circle.cpp
	sweep_edge.cpp
	sweep_interval.cpp
)
set(HEADERS
	circulator.h
	intersections.h
	node.h
	painting.h
	parameters.h
	place.h
	polar_line.h
	polar_point.h
	polar_segment.h
	reachable_region_algorithm.h
	spiral.h
	spiral_segment.h
	spiral_tree.h
	spiral_tree_obstructed_algorithm.h
	spiral_tree_unobstructed_algorithm.h
	sweep_circle.h
	sweep_edge.h
	sweep_interval.h
)

add_library(flow_map ${SOURCES})
target_link_libraries(flow_map
	PRIVATE glog::glog
)

cartocrow_install_module(flow_map)
install(FILES ${HEADERS} DESTINATION ${CARTOCROW_INSTALL_DIR}/flow_map)
