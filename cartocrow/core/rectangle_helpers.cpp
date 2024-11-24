#include "rectangle_helpers.h"

namespace cartocrow {
Corner opposite(Corner corner) {
	return static_cast<Corner>((corner + 2) % 4);
}

bool is_horizontal(Side side) {
	return side % 2 == 1;
}

Corner mirror_corner(Corner corner, bool vertical) {
	switch(corner) {
	case BL:
		if (vertical) return TL; else return BR;
	case BR:
		if (vertical) return TR; else return BL;
	case TR:
		if (vertical) return BR; else return TL;
	case TL:
		if (vertical) return BL; else return TR;
	}
}
}