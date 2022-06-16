#include "../catch.hpp"

#include "../../cartocrow/core/timer.h"

TEST_CASE("Creating a timer") {
	cartocrow::Timer timer;
	REQUIRE(timer.size() == 0);

	timer.stamp("test stamp");
	REQUIRE(timer.size() == 1);
	REQUIRE(timer[0].first == "test stamp");
	REQUIRE(timer[0].second >= 0);
	double firstDuration = timer[0].second;

	timer.stamp("another test stamp");
	REQUIRE(timer.size() == 2);
	REQUIRE(timer[0].first == "test stamp");
	REQUIRE(timer[0].second == firstDuration);
	REQUIRE(timer[1].first == "another test stamp");
	REQUIRE(timer[1].second >= 0);

	REQUIRE(timer.peek() >= 0);

	timer.reset();
	REQUIRE(timer.size() == 0);
}
