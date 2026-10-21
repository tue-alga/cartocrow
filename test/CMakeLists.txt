set(TEST_SOURCES "cartocrow_test.cpp"
	"core/cubic_bezier.cpp"
	"core/core.cpp"
	"core/kernel_conversions.cpp"
	"core/polygon_helpers.cpp"
	"core/region_arrangement.cpp"
	"core/region_map.cpp"
	"renderer/ipe_renderer.cpp"
	"reader/ipe_reader.cpp"
	"reader/gdal_reader.cpp"
	"reader/multi_reader.cpp"
	"reader/geojson_reader.cpp"
	"data_structures/graph_2.cpp"
)


add_executable(cartocrow_test cartocrow_test.cpp ${TEST_SOURCES})
target_link_libraries(cartocrow_test
	PRIVATE
	core
	renderer
	reader
)
