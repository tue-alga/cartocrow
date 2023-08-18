#include "../catch.hpp"

#include "cartocrow/necklace_map/range.h"

using namespace cartocrow::necklace_map;

TEST_CASE("Creating and copying ranges") {
	Range r1(2, 3);
	CHECK(r1.from() == 2);
	CHECK(r1.to() == 3);

	Range r2(r1);
	CHECK(r2.from() == 2);
	CHECK(r2.to() == 3);

	r2.from() = 1;
	CHECK(r2.from() == 1);
	CHECK(r2.to() == 3);
}

TEST_CASE("Checking if ranges are valid and degenerate") {
	Range r1(2, 3);
	CHECK(r1.isValid());
	CHECK(!r1.isDegenerate());

	Range r2(3, 3);
	CHECK(r2.isValid());
	CHECK(r2.isDegenerate());

	Range r3(4, 3);
	CHECK(!r3.isValid());
	CHECK(!r3.isDegenerate());
}

TEST_CASE("Checking if a range contains a given value") {
	Range r1(2, 4);

	CHECK(!r1.contains(1));
	CHECK(r1.contains(2));
	CHECK(r1.contains(3));
	CHECK(r1.contains(4));
	CHECK(!r1.contains(5));

	CHECK(!r1.containsInterior(1));
	CHECK(!r1.containsInterior(2));
	CHECK(r1.containsInterior(3));
	CHECK(!r1.containsInterior(4));
	CHECK(!r1.containsInterior(5));
}

TEST_CASE("Checking if ranges intersect") {
	Range r1(2, 4);
	Range r2(3, 5);
	Range r3(4, 6);
	Range r4(5, 7);

	CHECK(r1.intersects(r1));
	CHECK(r1.intersects(r2));
	CHECK(r1.intersects(r3));
	CHECK(!r1.intersects(r4));
	CHECK(r2.intersects(r1));
	CHECK(r3.intersects(r1));
	CHECK(!r4.intersects(r1));

	CHECK(r1.intersectsInterior(r1));
	CHECK(r1.intersectsInterior(r2));
	CHECK(!r1.intersectsInterior(r3));
	CHECK(!r1.intersectsInterior(r4));
	CHECK(r2.intersectsInterior(r1));
	CHECK(!r3.intersectsInterior(r1));
	CHECK(!r4.intersectsInterior(r1));
}
