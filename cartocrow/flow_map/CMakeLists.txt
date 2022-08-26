set(SOURCES
	intersections.cpp
	node.cpp
	parameters.cpp
	painting.cpp
	place.cpp
	polar_line.cpp
	polar_point.cpp
	polar_segment.cpp
	spiral.cpp
	spiral_segment.cpp
	spiral_tree.cpp
	spiral_tree_obstructed_algorithm.cpp
	spiral_tree_unobstructed_algorithm.cpp
	sweep_circle.cpp
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
	spiral.h
	spiral_segment.h
	spiral_tree.h
	spiral_tree_obstructed_algorithm.h
	spiral_tree_unobstructed_algorithm.h
	sweep_circle.h
)

add_library(flow_map ${SOURCES})
target_link_libraries(flow_map
	PRIVATE glog::glog
)

install(TARGETS flow_map
	EXPORT ${CARTOCROW_EXPORT_TARGETS}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
	INCLUDES DESTINATION ${CMAKE_INSTALL_SOURCE_DIR}
)
