set(TEST_SOURCES "cartocrow_test.cpp"
	"core/centroid.cpp"
	"core/core.cpp"
	"core/region_arrangement.cpp"
	"core/region_map.cpp"
	"core/timer.cpp"
	"necklace_map/bit_string.cpp"
	"necklace_map/circular_range.cpp"
	"necklace_map/necklace_map.cpp"
	"necklace_map/range.cpp"
	"renderer/ipe_renderer.cpp"
	"vw_simplification/vw_simplification.cpp"
)

add_executable(cartocrow_test cartocrow_test.cpp ${TEST_SOURCES})
target_link_libraries(cartocrow_test
	PRIVATE
	core
	necklace_map
	renderer
	vw_simplification
)
