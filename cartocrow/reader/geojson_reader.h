#pragma once

#include <nlohmann/json.hpp>

#include "linear_object_reader.h"
#include "../core/straight_geometry.h"

namespace cartocrow {
namespace {
using JSONObject = nlohmann::json;
}

template <class Geometry, class OutputIterator, class Traits>
concept GeoJSONReaderTraits =
    LinearObjectReaderTraits<
        JSONObject,
        Geometry,
        OutputIterator,
        Traits>;

using IntermediateGeoJSONGeometry = StraightGeometry<Inexact>;

template <class Geometry, class OutputIterator, class Traits>
concept GeoJSONReaderIntermediateGeometryConverter = requires(const IntermediateGeoJSONGeometry& g, OutputIterator out) {
	{ Traits::template convert<>(g, out) }->std::same_as<bool>;
};

Point<Inexact> parsePoint(const JSONObject& o) ;
MultiPoint<Inexact> parseMultiPoint(const JSONObject& o);
Polyline<Inexact> parseLineString(const JSONObject& o);
MultiPolyline<Inexact> parseMultiLineString(const JSONObject& o);
Polygon<Inexact> parseLinearRing(const JSONObject& o);
PolygonWithHoles<Inexact> parsePolygon(const JSONObject& o);
MultipolygonWithHoles<Inexact> parseMultiPolygon(const JSONObject& o);

/// is a model of GeoJSONReaderTraits
template<class Geometry, class Converter>
requires GeoJSONReaderIntermediateGeometryConverter<Geometry, std::back_insert_iterator<std::vector<Geometry>>, Converter>
struct GeoJSONReaderIntermediateGeometryTraits {
	// Accepts a geometry GeoJSON object
	template <class OutputIterator>
	static void convertToIntermediate(const JSONObject& geometry, OutputIterator out) {
		const auto& type = geometry["type"];

		if (type == "Point") {
			const auto& coordinates = geometry["coordinates"];
			*out++ = parsePoint(coordinates);
		} else if (type == "MultiPoint") {
			const auto& coordinates = geometry["coordinates"];
			*out++ = parseMultiPoint(coordinates);
		} else if (type == "LineString") {
			const auto& coordinates = geometry["coordinates"];
			*out++ = parseLineString(coordinates);
		} else if (type == "MultiLineString") {
			const auto& coordinates = geometry["coordinates"];
			*out++ = parseMultiLineString(coordinates);
		} else if (type == "Polygon") {
			const auto& coordinates = geometry["coordinates"];
			*out++ = parsePolygon(coordinates);
		} else if (type == "MultiPolygon") {
			const auto& coordinates = geometry["coordinates"];
			*out++ = parseMultiPolygon(coordinates);
		} else if (type == "GeometryCollection") {
			const auto& geometries = geometry["geometries"];
			for (const auto& subGeometry : geometries) {
				convertToIntermediate(subGeometry, out);
			}
		}

		return;
	}

	template <class OutputIterator>
	static bool convert(const JSONObject& o, OutputIterator out) {
		std::vector<IntermediateGeoJSONGeometry> intermediates;
		convertToIntermediate(o["geometry"], std::back_inserter(intermediates));

		for (const auto& intermediate : intermediates) {
			Converter::convert(intermediate, out);
		}

		return !intermediates.empty();
	}
};

// is a model of GeoJSONReaderIntermediateGeometryConverter
template <class Geometry>
struct BasicGeoJSONReaderTraitsConverter {
	template <class OutputIterator>
	static bool convert(const IntermediateGeoJSONGeometry& g, OutputIterator out) {
		bool convertedSomething = false;
		std::visit(
		    [&](auto&& g) {
			    using T = std::decay_t<decltype(g)>;

			    if constexpr (std::is_convertible_v<T, Geometry>) {
				    *out++ = Geometry{g};
				    convertedSomething = true;
			    }
		    },
		    g);
		return convertedSomething;
	}
};

template <class Geometry>
using BasicGeoJSONReaderTraits = GeoJSONReaderIntermediateGeometryTraits<Geometry, BasicGeoJSONReaderTraitsConverter<Geometry>>;

namespace {
using Out = std::back_insert_iterator<std::vector<Point<Inexact>>>;

static_assert(GeoJSONReaderTraits<Point<Inexact>, Out, BasicGeoJSONReaderTraits<Point<Inexact>>>);
}


class GeoJSONReader : public LinearObjectReader<JSONObject, BasicGeoJSONReaderTraits> {
  private:
	nlohmann::json m_json;

  public:
	GeoJSONReader(const std::filesystem::path& path) {
		std::ifstream file(path);
		m_json = nlohmann::json::parse(file);
	}

	GeoJSONReader(const std::string& jsonString) {
		m_json = nlohmann::json::parse(jsonString);
	}

	GeoJSONReader(const nlohmann::json& json) {
		m_json = json;
	}

  private:
	/// If handle returns true the parsing stops.
	void readHelper(std::function<bool(const JSONObject&)> handle) override {
		for (const auto& feature : m_json["features"]) {
			if (handle(feature)) {
				break;
			}
		}
	}

	GeometryAttributes getAttributes(const JSONObject& obj) const override {
		GeometryAttributes attributes;

		bool printedWarning = false;
		for (const auto& [property, value] : obj["properties"].items()) {
			if (value.is_string()) {
				attributes[property] = value.get<std::string>();
			} else if (value.is_boolean()) {
				attributes[property] = value.get<bool>();
			} else if (value.is_number_integer()) {
				attributes[property] = value.get<int>();
			} else if (value.is_number_float()) {
				attributes[property] = value.get<double>();
			} else if (value.is_null()) {
				continue; // currently we do not store null values
			} else {
				// arrays and objects are complicated because their values can have different types, just store it as a string for now
				// other types will also be stored as string
				if (!printedWarning) {
					std::cerr << "[Warning] Stored complex GeoJSON property of type " << value.type_name() << " as string!" << std::endl;
					printedWarning = true;
				}
				attributes[property] = to_string(value);
				continue;
			}
		}

		return attributes;
	}

	bool skipObject(const JSONObject& obj) const override {
		return false;
	}
  public:
	// ===== Reader methods =====
	/// Load a different file.
 	void load(const std::filesystem::path& path) {
		std::ifstream file(path);
		m_json = nlohmann::json::parse(file);
	}

	/// Load a different JSON string.
	void load(const std::string& jsonString) {
		m_json = nlohmann::json::parse(jsonString);
	}

	/// Load a different JSON object.
	void load(const nlohmann::json& json) {
		m_json = json;
	}

	/// Currently we do not support GeoJSON with a CRS.
	std::optional<std::string> readSpatialReference() {
		return std::nullopt;
	}

	/// Returns whether the reader can parse the given file.
	static bool canRead(std::filesystem::path path) {
		return path.extension() == ".geojson";
	}
};

namespace {
static_assert(GeometryReader<GeoJSONReader>);
static_assert(GeometryReaderFor<GeoJSONReader, Point<Inexact>>);
}
}