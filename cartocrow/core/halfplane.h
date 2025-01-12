#ifndef CARTOCROW_HALFPLANE_H
#define CARTOCROW_HALFPLANE_H

#include <CGAL/Line_2.h>
#include <CGAL/Polygon_2.h>
#include "rectangle_helpers.h"
#include "core.h"

namespace cartocrow {
template <class K> class Halfplane {
  private:
	CGAL::Line_2<K> m_line;

  public:
	Halfplane(CGAL::Line_2<K> line) : m_line(std::move(line)) {}
	CGAL::Line_2<K> line() const {
		return m_line;
	}

	CGAL::Oriented_side oriented_side(const Point<K>& p) const {
		return m_line.oriented_side(p);
	}

	CGAL::Bounded_side bounded_side(const Point<K>& p) const {
		return oriented_side(p);
	}

	bool has_on_positive_side(const Point<K>& p) const {
		return oriented_side(p) == CGAL::ON_POSITIVE_SIDE;
	}

	bool has_on_negative_side(const Point<K>& p) const {
		return oriented_side(p) == CGAL::ON_NEGATIVE_SIDE;
	}

	bool has_on_boundary(const Point<K>& p) const {
		return oriented_side(p) == CGAL::ON_BOUNDARY;
	}

	bool has_on_bounded_side(const Point<K>& p) const {
		return has_on_positive_side(p);
	}

	bool has_on_unbounded_side(const Point<K>& p) const {
		return has_on_negative_side(p);
	}

	CGAL::Polygon_2<Inexact> polygon(const Rectangle<Inexact>& rect) const {
		CGAL::Polygon_2<Inexact> poly;
		auto result = intersection(
		    m_line, CGAL::Iso_rectangle_2<Inexact>(Point<Inexact>(rect.xmin(), rect.ymin()),
		                                           Point<Inexact>(rect.xmax(), rect.ymax())));

		if (result) {
			if (const Segment<Inexact>* s = boost::get<Segment<Inexact>>(&*result)) {
				poly.push_back(s->source());
				poly.push_back(s->target());

				auto boundsSide = [&rect](const Point<Inexact>& p) {
					if (abs(p.y() - rect.ymax()) < M_EPSILON) {
						return Top;
					} else if (abs(p.y() - rect.ymin()) < M_EPSILON) {
						return Bottom;
					} else if (abs(p.x() - rect.xmin()) < M_EPSILON) {
						return Left;
					} else if (abs(p.x() - rect.xmax()) < M_EPSILON) {
						return Right;
					} else {
						throw std::runtime_error(
						    "Endpoint of line-rectangle intersection do not lie on the rectangle.");
					}
				};

				auto sourceSide = boundsSide(s->source());
				auto targetSide = boundsSide(s->target());
				auto current = targetSide;
				while (current != sourceSide) {
					auto next = next_side(current);
					poly.push_back(get_corner<Inexact>(rect, current, next));
					current = next_side(current);
				}
			}
		}
		return poly;
	}

	CGAL::Polygon_2<Exact> polygon(const Rectangle<Exact>& rect) const {
		CGAL::Polygon_2<Exact> poly;
		auto result = intersection(
		    m_line, CGAL::Iso_rectangle_2<Exact>(Point<Exact>(rect.xmin(), rect.ymin()),
		                                           Point<Exact>(rect.xmax(), rect.ymax())));

		if (result) {
			if (const Segment<Exact>* s = boost::get<Segment<Exact>>(&*result)) {
				poly.push_back(s->source());
				poly.push_back(s->target());

				auto boundsSide = [&rect](const Point<Exact>& p) {
					if (p.y() == rect.ymax()) {
						return Top;
					} else if (p.y() == rect.ymin()) {
						return Bottom;
					} else if (p.x() == rect.xmin()) {
						return Left;
					} else if (p.x() == rect.xmax()) {
						return Right;
					} else {
						throw std::runtime_error(
						    "Endpoint of line-rectangle intersection do not lie on the rectangle.");
					}
				};

				auto sourceSide = boundsSide(s->source());
				auto targetSide = boundsSide(s->target());
				auto current = targetSide;
				while (current != sourceSide) {
					auto next = next_side(current);
					poly.push_back(get_corner<Exact>(rect, current, next));
					current = next_side(current);
				}
			}
		}

		return poly;
	}
};

/// Converts a halfplane from exact representation to an approximation in
/// inexact representation.
template <class K>
Halfplane<Inexact> approximate(const Halfplane<K>& p) {
	return {approximate(p.line())};
}
}

#endif //CARTOCROW_HALFPLANE_H
