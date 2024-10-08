#ifndef CARTOCROW_POLYPATTERN_H
#define CARTOCROW_POLYPATTERN_H

#include "pattern.h"

namespace cartocrow::simplesets {
template <class... Args>
struct variant_cast_proxy
{
	std::variant<Args...> v;

	template <class... ToArgs>
	operator std::variant<ToArgs...>() const
	{
		return std::visit([](auto&& arg) -> std::variant<ToArgs...> { return arg ; },
		                  v);
	}
};

template <class... Args>
auto variant_cast(const std::variant<Args...>& v) -> variant_cast_proxy<Args...>
{
	return {v};
}

class PolyPattern : public Pattern {
  public:
	virtual std::variant<Polyline<Inexact>, Polygon<Inexact>> poly() const = 0;
	std::variant<Polyline<Inexact>, Polygon<Inexact>, CSPolygon> contour() const override {
	    return variant_cast(poly());
	};
	virtual Number<Inexact> coverRadius() const = 0;
};
}

#endif //CARTOCROW_POLYPATTERN_H
