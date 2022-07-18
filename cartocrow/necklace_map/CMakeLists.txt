set(SOURCES
	check_feasible/check_feasible_exact.cpp
	check_feasible/check_feasible_heuristic.cpp
	check_feasible/check_feasible.cpp
	detail/cycle_node_layered.cpp
	detail/cycle_node.cpp
	detail/task.cpp
	detail/validate_scale_factor.cpp
	feasible_interval/compute_feasible_interval_centroid.cpp
	feasible_interval/compute_feasible_interval_wedge.cpp
	feasible_interval/compute_feasible_interval.cpp
	bead.cpp
	bezier.cpp
	bezier_necklace.cpp
	circle_necklace.cpp
	circular_range.cpp
	necklace.cpp
	necklace_interval.cpp
	necklace_map.cpp
	necklace_shape.cpp
	parameters.cpp
	polar_line.cpp
	polar_point.cpp
	polar_segment.cpp
	range.cpp
)
set(HEADERS
	check_feasible/check_feasible_exact.h
	check_feasible/check_feasible_heuristic.h
	check_feasible/check_feasible.h
	detail/cycle_node_layered.h
	detail/cycle_node.h
	detail/task.h
	detail/validate_scale_factor.h
	feasible_interval/compute_feasible_interval_centroid.h
	feasible_interval/compute_feasible_interval_wedge.h
	feasible_interval/compute_feasible_interval.h
	bead.h
	bezier.h
	bezier_necklace.h
	bit_string.h
	circle_necklace.h
	circular_range.h
	necklace.h
	necklace_interval.h
	necklace_map.h
	necklace_shape.h
	parameters.h
	polar_line.h
	polar_point.h
	polar_segment.h
	range.h
)

add_library(necklace_map ${SOURCES})
target_link_libraries(necklace_map
	PUBLIC Ipe::ipelib
	PRIVATE glog::glog
)

install(TARGETS necklace_map
	EXPORT ${CARTOCROW_EXPORT_TARGETS}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
	INCLUDES DESTINATION ${CMAKE_INSTALL_SOURCE_DIR}
)

# install_files_and_directories("${HEADERS}" ${CARTOCROW_INSTALL_DIR}/necklace_map)
