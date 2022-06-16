file(GLOB_RECURSE TEST_SOURCES "**/*.cpp")

add_executable(cartocrow_test cartocrow_test.cpp ${TEST_SOURCES})
target_link_libraries(cartocrow_test core)
