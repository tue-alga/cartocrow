#include "../catch.hpp"

#include <ctime>

#include "cartocrow/core/timer.h"

TEST_CASE("Creating and using a timer") {
	clock_t startTime = clock();
	cartocrow::Timer timer;
	REQUIRE(timer.size() == 0);

	while (double(clock() - startTime) / CLOCKS_PER_SEC < 0.01) {
		// busy wait
	}
	timer.stamp("Test stamp");
	REQUIRE(timer.size() == 1);
	CHECK(timer[0].first == "Test stamp");
	CHECK(timer[0].second == Approx(0.01).epsilon(0.01));
	double firstDuration = timer[0].second;

	while (double(clock() - startTime) / CLOCKS_PER_SEC < 0.03) {
		// busy wait
	}
	timer.stamp("Another test stamp");
	REQUIRE(timer.size() == 2);
	CHECK(timer[0].first == "Test stamp");
	CHECK(timer[0].second == firstDuration);
	CHECK(timer[1].first == "Another test stamp");
	CHECK(timer[1].second == Approx(0.02).epsilon(0.01));

	while (double(clock() - startTime) / CLOCKS_PER_SEC < 0.06) {
		// busy wait
	}
	REQUIRE(timer.size() == 2);
	CHECK(timer.peek() == Approx(0.03).epsilon(0.01));

	timer.reset();
	REQUIRE(timer.size() == 0);
}
