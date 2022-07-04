set(TEST_SOURCES "cartocrow_test.cpp"
	"core/core.cpp"
	"core/polygon.cpp"
	"core/region_map.cpp"
	"core/timer.cpp"
	"renderer/ipe_renderer.cpp"
)

add_executable(cartocrow_test cartocrow_test.cpp ${TEST_SOURCES})
target_link_libraries(cartocrow_test core renderer)
