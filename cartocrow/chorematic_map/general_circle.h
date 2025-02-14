#ifndef CARTOCROW_GENERAL_CIRCLE_H
#define CARTOCROW_GENERAL_CIRCLE_H

#include "../core/core.h"
#include "../core/halfplane.h"

namespace cartocrow::chorematic_map {
template <class K>
class GeneralCircle {
  private:
	std::variant<Circle<K>, Halfplane<K>> m_rep;

  public:
	GeneralCircle(Circle<K> circle) : m_rep(std::move(circle)) {};
	GeneralCircle(Halfplane<K> halfplane) : m_rep(std::move(halfplane)) {};

	bool is_circle() const {
		return m_rep.index() == 0;
	}
	Circle<K> get_circle() const {
	    return get<Circle<K>>(m_rep);
	}
	bool is_halfplane() const {
		return m_rep.index() == 1;
	}
	Halfplane<K> get_halfplane() const {
		return get<Halfplane<K>>(m_rep);
	}
	CGAL::Oriented_side oriented_side(const Point<K>& p) const {
		return std::visit([&p](const auto& v) { return v.oriented_side(p); }, m_rep);
	}
	CGAL::Bounded_side bounded_side(const Point<K>& p) const {
		return std::visit([&p](const auto& v) { return v.bounded_side(p); }, m_rep);
	}
	bool has_on_positive_side(const Point<K>& p) const {
		return std::visit([&p](const auto& v) { return v.has_on_positive_side(p); }, m_rep);
	}
	bool has_on_negative_side(const Point<K>& p) const {
		return std::visit([&p](const auto& v) { return v.has_on_negative_side(p); }, m_rep);
	}
	bool has_on_boundary(const Point<K>& p) const {
		return std::visit([&p](const auto& v) { return v.has_on_boundary(p); }, m_rep);
	}
	bool has_on_bounded_side(const Point<K>& p) const {
		return std::visit([&p](const auto& v) { return v.has_on_bounded_side(p); }, m_rep);
	}
	bool has_on_unbounded_side(const Point<K>& p) const {
		return std::visit([&p](const auto& v) { return v.has_on_unbounded_side(p); }, m_rep);
	}
};
}

#endif //CARTOCROW_GENERAL_CIRCLE_H
