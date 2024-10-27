#ifndef CARTOCROW_CAVC_HELPERS_H
#define CARTOCROW_CAVC_HELPERS_H

#include "../types.h"
#include "cavc/include/cavc/polylineoffset.hpp"

namespace cartocrow::simplesets {

//std::vector<X_monotone_curve_2> xmCurves(const cavc::Polyline<double>& polyline);

//std::variant<CSPolyline, CSPolygon> toCSPoly(const cavc::Polyline<double>& polyline);

CSPolygonSet approximateDilate(const CSPolygonSet& csPolygonSet, double radius);
CSPolygonSet approximateErode(const CSPolygonSet& csPolygonSet, double radius);
CSPolygonSet approximateSmooth(const CSPolygonSet& csPolygonSet, double radius);
CSPolygonSet approximateSmooth_(const CSPolygonSet& polygonSet, double radius);

cavc::Polyline<double> cavcPolyline(const CSPolygon& polygon);
cavc::Polyline<double> cavcPolyline(const CSPolyline& polyline);
}
#endif //CARTOCROW_CAVC_HELPERS_H
