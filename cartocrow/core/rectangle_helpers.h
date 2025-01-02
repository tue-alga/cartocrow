#ifndef CARTOCROW_RECTANGLE_HELPERS_H
#define CARTOCROW_RECTANGLE_HELPERS_H

#include "core.h"

namespace cartocrow {
enum Side {
	Left,
	Bottom,
	Right,
	Top
};

enum Corner {
	BL,
	BR,
	TR,
	TL,
};

Corner opposite(Corner corner);

bool is_horizontal(Side side);

Corner mirror_corner(Corner corner, bool vertical);

template <class K>
Number<K> width(const Rectangle<K>& rect) {
	return rect.xmax() - rect.xmin();
}

template <class K>
Number<K> height(const Rectangle<K>& rect) {
	return rect.ymax() - rect.ymin();
}

template <class K>
Point<K> centroid(const Rectangle<K>& rect) {
	return {(rect.xmin() + rect.xmax()) / 2, (rect.ymin() + rect.ymax()) / 2};
}

template <class K>
auto dimension(const Rectangle<K>& rect, int i) {
	if (i == 0) {
		return width(rect);
	} else if (i == 1) {
		return height(rect);
	} else {
		throw std::runtime_error("Dimension i is not 0 or 1");
	}
}

template <class K>
Corner corner(const Rectangle<K>& rect, const Side& side1, const Side& side2) {
	if (side1 > side2) return corner(rect, side2, side1);
	int dist = side2 - side1;
	if (dist == 1) {
		return static_cast<Corner>(side1);
	} else if (dist == 3) {
		return static_cast<Corner>(side2);
	} else {
		throw std::runtime_error("Sides are not adjacent");
	}
}

template <class K>
Point<K> get_corner(const Rectangle<K>& rect, const Corner& corner) {
	return rect.vertex(static_cast<int>(corner));
}

template <class K>
Point<K> get_corner(const Rectangle<K>& rect, const Side& side1, const Side& side2) {
	Corner c = corner(rect, side1, side2);
	return get_corner(rect, c);
}

template <class K>
Side side(const Rectangle<K>& rect, Corner corner1, Corner corner2) {
	if (corner1 > corner2) return side(rect, corner2, corner1);
	int dist = corner2 - corner1;
	if (dist == 1) {
		return static_cast<Side>(corner2);
	} else if (dist == 3) {
		return static_cast<Side>(corner1);
	} else {
		throw std::runtime_error("Corners are not adjacent");
	}
}

template <class K>
Segment<K> get_side(const Rectangle<K>& rect, Side side) {
	int i = static_cast<int>(side);
	return {rect.vertex((i+3)%4), rect.vertex(i)};
}

template <class K>
Segment<K> get_side(const Rectangle<K>& rect, Corner corner1, Corner corner2) {
	return get_side(side(rect, corner1, corner2));
}

template <class K>
Side closest_side(const Point<K>& point, const Rectangle<K>& bb) {
	std::vector<double> dist({ CGAL::to_double(point.x()) - CGAL::to_double(bb.xmin()), CGAL::to_double(point.y()) - CGAL::to_double(bb.ymin()),
	                           CGAL::to_double(bb.xmax()) - CGAL::to_double(point.x()), CGAL::to_double(bb.ymax()) - CGAL::to_double(point.y()) });
	auto it = std::min_element(dist.begin(), dist.end());
	return static_cast<Side>(std::distance(dist.begin(), it));
}

template <class K>
Vector<K> side_direction(const Side& side) {
	switch (side) {
	case Left:
		return { -1, 0 };
	case Top:
		return { 0, 1 };
	case Right:
		return { 1, 0 };
	case Bottom:
		return { 0, -1 };
	default:
		throw std::runtime_error("Impossible");
	}
}

template <class K>
Point<K> proj_on_side(Point<K> p, Side side, const Rectangle<K>& rect) {
	switch (side) {
	case Left:
		return { rect.xmin(), p.y() };
	case Top:
		return { p.x(), rect.ymax() };
	case Right:
		return { rect.xmax(), p.y() };
	case Bottom:
		return { p.x(), rect.ymin() };
	default:
		throw std::runtime_error("Impossible");
	}
}

Side next_side(const Side& side);
}

#endif //CARTOCROW_RECTANGLE_HELPERS_H
