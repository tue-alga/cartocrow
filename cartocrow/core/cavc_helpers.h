#ifndef CARTOCROW_CAVC_HELPERS_H
#define CARTOCROW_CAVC_HELPERS_H

#include "cs_types.h"
#include <cavc/polylineoffset.hpp>

namespace cartocrow {
/// Approximately dilate a CSPolygonSet.
/// That is, return the approximate Minkowski sum of the provided CSPolygonSet with a disk of the provided radius.
CSPolygonSet approximateDilate(const CSPolygonSet& csPolygonSet, double radius);
/// Erode a CSPolygonSet.
/// That is, return the approximate Minkowski difference of the provided CSPolygonSet with a disk of the provided radius.
CSPolygonSet approximateErode(const CSPolygonSet& csPolygonSet, double radius);
/// Perform the opening operator on the CSPolygonSet.
/// That is, first erode then dilate with a disk of the provided radius.
CSPolygonSet approximateOpening(const CSPolygonSet& csPolygonSet, Number<Exact> radius);
/// Perform the closing operator on the CSPolygonSet.
/// That is, first dilate then erode with a disk of the provided radius.
CSPolygonSet approximateClosing(const CSPolygonSet& csPolygonSet, Number<Exact> radius);
/// Smooth a CSPolygonSet by first applying the closing operator and then the opening operator.
CSPolygonSet approximateSmoothCO(const CSPolygonSet& csPolygonSet, double radius);
/// Smooth a CSPolygonSet by first applying the opening operator and then the closing operator.
CSPolygonSet approximateSmoothOC(const CSPolygonSet& csPolygonSet, double radius);
/// Approximately dilate a CSPolygonSet.
/// That is, return the approximate Minkowski sum of the provided CSPolygonSet with a disk of the provided radius.
CSPolygonSet approximateDilate(const CSPolygonSet& csPolygonSet, Number<Exact> radius);
/// Erode a CSPolygonSet.
/// That is, return the approximate Minkowski difference of the provided CSPolygonSet with a disk of the provided radius.
CSPolygonSet approximateErode(const CSPolygonSet& csPolygonSet, Number<Exact> radius);
/// Perform the opening operator on the CSPolygonSet.
/// That is, first erode then dilate with a disk of the provided radius.
CSPolygonSet approximateOpening(const CSPolygonSet& csPolygonSet, Number<Exact> radius);
/// Perform the closing operator on the CSPolygonSet.
/// That is, first dilate then erode with a disk of the provided radius.
CSPolygonSet approximateClosing(const CSPolygonSet& csPolygonSet, Number<Exact> radius);
/// Smooth a CSPolygonSet by first applying the closing operator and then the opening operator.
CSPolygonSet approximateSmoothCO(const CSPolygonSet& csPolygonSet, Number<Exact> radius);
/// Smooth a CSPolygonSet by first applying the opening operator and then the closing operator.
CSPolygonSet approximateSmoothOC(const CSPolygonSet& csPolygonSet, Number<Exact> radius);

cavc::Polyline<double> cavcPolyline(const CSPolygon& polygon);
cavc::Polyline<double> cavcPolyline(const CSPolyline& polyline);
}
#endif //CARTOCROW_CAVC_HELPERS_H
