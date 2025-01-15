#include "input_parsing.h"

namespace cartocrow::chorematic_map {
std::shared_ptr<std::unordered_map<std::string, RegionWeight>>
regionDataMapFromGPKG(const std::filesystem::path& path, const std::string& layerName, const std::string& regionNameAttribute,
                      const std::function<std::string(std::string)>& regionNameTransform) {
    GDALAllRegister();
    GDALDataset *poDS;

    poDS = (GDALDataset*) GDALOpenEx( path.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr );
    if( poDS == nullptr )
    {
        printf( "Open failed.\n" );
        exit( 1 );
    }
    OGRLayer* poLayer = poDS->GetLayerByName( layerName.c_str() );

    auto regionDataMap = std::make_shared<std::unordered_map<std::string, RegionWeight>>();
    std::unordered_map<std::string, RegionWeight>& dataMap = *regionDataMap;
    poLayer->ResetReading();

    for (auto& poFeature : *poLayer) {
        std::string regionId = regionNameTransform(poFeature->GetFieldAsString(poFeature->GetFieldIndex(regionNameAttribute.c_str())));
        int i = 0;
        for( auto&& oField: *poFeature ) {
            std::string name = poFeature->GetDefnRef()->GetFieldDefn(i)->GetNameRef();
            std::optional<double> w;
            switch(oField.GetType()) {
                case OFTInteger:
                    w = oField.GetInteger();
                    break;
                case OFTReal:
                    w = oField.GetDouble();
                    break;
			    default:
//				    std::cerr << "Field " << name << " for region " << regionId << " neither an integer nor double!" << std::endl;
				    break;
            }
            if (w.has_value() && w != -99999999) {
                dataMap[name][regionId] = *w;
            }
            ++i;
        }
    }

    return regionDataMap;
}

std::shared_ptr<RegionMap> regionMapFromGPKG(const std::filesystem::path& path,
                                             const std::string& layerName,
                                             const std::string& regionNameAttribute,
                                             const std::optional<std::function<bool(const OGRFeature&)>>& skip) {
    GDALAllRegister();
    GDALDataset       *poDS;

    poDS = (GDALDataset*) GDALOpenEx( path.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr );
    if( poDS == nullptr )
    {
        printf( "Open failed.\n" );
        exit( 1 );
    }
    OGRLayer* poLayer = poDS->GetLayerByName( layerName.c_str() );

    auto regionMap = std::make_shared<RegionMap>();
    RegionMap& regions = *regionMap;
    poLayer->ResetReading();

    for (auto& poFeature : *poLayer) {
        if (skip.has_value() && (*skip)(*poFeature)) continue;
        std::string regionId = poFeature->GetFieldAsString(poFeature->GetFieldIndex(regionNameAttribute.c_str()));
        OGRGeometry *poGeometry;

        PolygonSet<Exact> polygonSet;
        poGeometry = poFeature->GetGeometryRef();
        if( wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPolygon ) {
            OGRMultiPolygon *poMultiPolygon = poGeometry->toMultiPolygon();

            for (auto &poly: *poMultiPolygon) {
                for (auto &linearRing: *poly) {
                    Polygon<Exact> polygon;
                    for (auto &pt: *linearRing) {
                        polygon.push_back({pt.getX(), pt.getY()});
                    }
                    // if the begin and end vertices are equal, remove one of them
                    if (polygon.container().front() == polygon.container().back()) {
                        polygon.container().pop_back();
                    }
                    if (polygon.is_clockwise_oriented()) {
                        polygon.reverse_orientation();
                    }
                    polygonSet.symmetric_difference(polygon);
                }
            }
        } else if (wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon) {
            OGRPolygon* poly = poGeometry->toPolygon();

            for (auto& linearRing : *poly) {
                Polygon<Exact> polygon;
                for (auto& pt : *linearRing) {
                    polygon.push_back({pt.getX(), pt.getY()});
                }
                // if the begin and end vertices are equal, remove one of them
                if (polygon.container().front() == polygon.container().back()) {
                    polygon.container().pop_back();
                }
                if (polygon.is_clockwise_oriented()) {
                    polygon.reverse_orientation();
                }
                polygonSet.symmetric_difference(polygon);
            }
        } else {
            std::cout << "Did not handle this type of geometry: " << poGeometry->getGeometryName() << std::endl;
        }
        if (regions.contains(regionId)) {
            Region& existingRegion = regions.at(regionId);
            PolygonSet<Exact>& existingPolygonSet = existingRegion.shape;
            existingPolygonSet.join(polygonSet);
        } else {
            Region region;
            region.shape = polygonSet;
            region.name = regionId;
            regions[regionId] = region;
        }
    }

    return regionMap;
}
}