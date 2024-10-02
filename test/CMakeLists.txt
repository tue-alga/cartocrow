set(TEST_SOURCES "cartocrow_test.cpp"
	"core/centroid.cpp"
	"core/core.cpp"
	"core/region_arrangement.cpp"
	"core/region_map.cpp"
	"core/timer.cpp"
	"flow_map/intersections.cpp"
	"flow_map/polar_line.cpp"
	"flow_map/polar_point.cpp"
	"flow_map/polar_segment.cpp"
	"flow_map/reachable_region_algorithm.cpp"
	"flow_map/spiral_tree.cpp"
	"flow_map/spiral_tree_obstructed_algorithm.cpp"
	"flow_map/sweep_circle.cpp"
	"flow_map/sweep_edge.cpp"
	"necklace_map/bit_string.cpp"
	"necklace_map/circular_range.cpp"
	"necklace_map/necklace_map.cpp"
	"necklace_map/range.cpp"
	"renderer/ipe_renderer.cpp"
	"simplification/vw_simplification.cpp"
	"simplesets/poly_line_gon_intersection.cpp"
	"simplesets/partition_algorithm.cpp"
	"simplesets/collinear_island.cpp"
)

add_executable(cartocrow_test cartocrow_test.cpp ${TEST_SOURCES})
target_link_libraries(cartocrow_test
	PRIVATE
	core
	flow_map
	necklace_map
	renderer
	simplification
	simplesets
)
