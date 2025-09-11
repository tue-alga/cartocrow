#include "input_parsing.h"
#include "cartocrow/reader/gdal_conversion.h"

namespace cartocrow::chorematic_map {
std::shared_ptr<std::unordered_map<std::string, RegionWeight>>
regionDataMapFromGPKG(const std::filesystem::path& path, const std::string& regionNameAttribute, const std::optional<std::string>& layerName,
                      const std::function<std::string(std::string)>& regionNameTransform) {
    GDALAllRegister();
    GDALDataset *poDS;

    poDS = (GDALDataset*) GDALOpenEx( path.string().c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr );
    if( poDS == nullptr )
    {
        printf( "Open failed.\n" );
        exit( 1 );
    }

	OGRLayer* poLayer;
	if (layerName.has_value()) {
    	poLayer = poDS->GetLayerByName( layerName->c_str() );
	} else {
		poLayer = poDS->GetLayer(0);
        if (poDS->GetLayerCount() > 1) {
            std::cout << "Reading first layer: " << poLayer->GetName() << std::endl;
        }
	}

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
                                             const std::string& regionNameAttribute,
											 const std::optional<std::string>& layerName,
                                             const std::optional<std::function<bool(const OGRFeature&)>>& skip) {
    GDALAllRegister();
    GDALDataset       *poDS;

    poDS = (GDALDataset*) GDALOpenEx( path.string().c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr );
    if( poDS == nullptr )
    {
        printf( "Open failed.\n" );
        exit( 1 );
    }
    OGRLayer* poLayer;
	if (layerName.has_value()) {
		poLayer = poDS->GetLayerByName( layerName->c_str() );
	} else {
		poLayer = poDS->GetLayer(0);
	}

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
            polygonSet = ogrMultiPolygonToPolygonSet(*poMultiPolygon);
        } else if (wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon) {
            OGRPolygon* poly = poGeometry->toPolygon();
            polygonSet = ogrPolygonToPolygonSet(*poly);
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

std::string getNextLine(std::istream& str) {
    std::string line;
    std::getline(str,line);
    return line;
}

std::vector<std::string> splitIntoTokens(const std::string& line, char delimiter) {
    std::vector<std::string>   result;
    std::stringstream          lineStream(line);
    std::string                cell;

    while(std::getline(lineStream, cell, delimiter))
    {
        result.push_back(cell);
    }
    // This checks for a trailing comma with no data after it.
    if (!lineStream && cell.empty())
    {
        // If there was a trailing comma then add an empty element.
        result.emplace_back("");
    }
    return result;
}

std::unordered_map<std::string, double> parseRegionData(const std::string& s, char delimiter) {
    std::stringstream ss(s);

    std::unordered_map<std::string, double> result;

    while (ss) {
        auto parts = splitIntoTokens(getNextLine(ss), delimiter);
        if (parts.size() <= 1) break;
        if (parts.size() != 2) {
            throw std::runtime_error("Input has incorrect format.");
        }
        result[parts[0]] = stod(parts[1]);
    }

    return result;
}

std::string regionDataToCSV(const std::unordered_map<std::string, double>& regionData) {
    std::stringstream ss;
    for (const auto& [region, value] : regionData) {
        ss << region << "," << value << "\n";
    }
    return ss.str();
}
}