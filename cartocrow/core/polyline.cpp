#include "polyline.h"

namespace cartocrow {
Polyline<Exact> pretendExact(const Polyline<Inexact>& p) {
	Polyline<Exact> result;
	for (auto v = p.vertices_begin(); v < p.vertices_end(); ++v) {
		result.push_back(pretendExact(*v));
	}
	return result;
}
}