#include "transform_helpers.h"
#include "rectangle_helpers.h"

namespace cartocrow {
PolygonWithHoles<Inexact> transform(const CGAL::Aff_transformation_2 <Inexact> &t, const PolygonWithHoles <Inexact> &pwh) {
    Polygon<Inexact> outerT;
    if (!pwh.is_unbounded()) {
        outerT = transform(t, pwh.outer_boundary());
    }
    std::vector <Polygon<Inexact>> holesT;
    for (const auto &h: pwh.holes()) {
        holesT.push_back(transform(t, h));
    }
    return {outerT, holesT.begin(), holesT.end()};
}

CGAL::Aff_transformation_2<Inexact> fitInto(const Rectangle<Inexact>& toFit, const Rectangle<Inexact>& into) {
    CGAL::Aff_transformation_2<Inexact> move1(CGAL::TRANSLATION, CGAL::ORIGIN - centroid(toFit));
    CGAL::Aff_transformation_2<Inexact> move2(CGAL::TRANSLATION, centroid(into) - CGAL::ORIGIN);
    CGAL::Aff_transformation_2<Inexact> scale(CGAL::SCALING, std::min(width(into) / width(toFit), height(into) / height(toFit)));
    return move2 * scale * move1;
}
}