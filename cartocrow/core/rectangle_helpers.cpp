#include "rectangle_helpers.h"

namespace cartocrow {
Corner opposite(Corner corner) {
	return static_cast<Corner>((corner + 2) % 4);
}

bool is_horizontal(Side side) {
	return side % 2 == 1;
}

Corner mirror_corner(Corner corner, bool vertical) {
	Corner mirrored;
	switch(corner) {
	case BL:
		mirrored = vertical ? TL : BR;
		break;
	case BR:
		mirrored = vertical ? TR : BL;
		break;
	case TR:
		mirrored = vertical ? BR : TL;
		break;
	case TL:
		mirrored = vertical ? BL : TR;
		break;
	}
	return mirrored;
}

Side next_side(const Side& side) {
	return static_cast<Side>((side + 1) % 4);
}
}