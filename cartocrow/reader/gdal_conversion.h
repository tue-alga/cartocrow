#ifndef CARTOCROW_GDAL_CONVERSION_H
#define CARTOCROW_GDAL_CONVERSION_H

#include <gdal/ogrsf_frmts.h>
#include "cartocrow/core/core.h"

namespace cartocrow {
PolygonSet<Exact> ogrMultiPolygonToPolygonSet(const OGRMultiPolygon& multiPolygon);
PolygonSet<Exact> ogrPolygonToPolygonSet(const OGRPolygon& ogrPolygon);
Polygon<Exact> ogrLinearRingToPolygon(const OGRLinearRing& ogrLinearRing);
PolygonWithHoles<Exact> ogrPolygonToPolygonWithHoles(const OGRPolygon& ogrPolygon);
OGRLinearRing polygonToOGRLinearRing(const Polygon<Exact>& polygon);
OGRPolygon polygonWithHolesToOGRPolygon(const PolygonWithHoles<Exact>& polygon);
OGRMultiPolygon polygonSetToOGRMultiPolygon(const PolygonSet<Exact>& polygonSet);
}

#endif //CARTOCROW_GDAL_CONVERSION_H
