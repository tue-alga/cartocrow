#include "../catch.hpp"

#include "cartocrow/necklace_map/bit_string.h"

using namespace cartocrow::necklace_map;

TEST_CASE("Creating and manipulating bit strings") {
	BitString bits;
	CHECK(bits.isEmpty());

	REQUIRE(bits.checkFit(5));
	CHECK(bits.get() == 0b000000);
	bits += 3;
	CHECK(bits.get() == 0b001000);
	bits += 5;
	CHECK(bits.get() == 0b101000);
	bits += 5;
	CHECK(bits.get() == 0b101000);
	bits -= 5;
	CHECK(bits.get() == 0b001000);
	bits -= 3;
	CHECK(bits.get() == 0b000000);

	bits ^= BitString::fromString(0b101010);
	CHECK(bits.get() == 0b101010);
	bits &= BitString::fromString(0b001111);
	CHECK(bits.get() == 0b001010);
	CHECK(!bits.isEmpty());
}
