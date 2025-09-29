#include <iostream>

#include "cartocrow/simplesets/parse_input.h"
#include "cartocrow/simplesets/settings.h"
#include "cartocrow/simplesets/partition_algorithm.h"
#include "cartocrow/simplesets/drawing_algorithm.h"

#include "cartocrow/renderer/svg_renderer.h"

#include <emscripten/bind.h>

#include <nlohmann/json.hpp>

using namespace emscripten;

using namespace cartocrow;
using namespace cartocrow::renderer;

using json = nlohmann::json;

void createSvg(std::filesystem::path projectFilename, std::filesystem::path outputFilename) {
	std::ifstream f(projectFilename);
	json projectData = json::parse(f);

	// Parse points
	auto filePath = projectFilename.parent_path() / projectData["points"];
	std::ifstream inputStream(filePath, std::ios_base::in);
	if (!inputStream.good()) {
		throw std::runtime_error("Failed to read input");
	}
	std::stringstream buffer;
	buffer << inputStream.rdbuf();
	auto points = simplesets::parseCatPoints(buffer.str());

	// Parse settings
	const auto& pd = projectData;

	simplesets::GeneralSettings gs;
	const auto& pdgs = pd["generalSettings"];
	gs.pointSize = pdgs["pointSize"];
	gs.inflectionLimit = pdgs["inflectionLimit"];
	gs.maxBendAngle = pdgs["maxBendAngle"];
	gs.maxTurnAngle = pdgs["maxTurnAngle"];

	simplesets::DrawSettings ds;
	const auto& pdds = pd["drawSettings"];
	auto pdColors = pdds["colors"];
	std::vector<Color> colors;
	for (const auto& entry : pdColors) {
		std::string hexString = entry.get<std::string>();
		colors.emplace_back(std::strtol(hexString.c_str(), nullptr, 0));
	}
	ds.colors = colors;
	ds.whiten = pdds["whiten"];

	simplesets::PartitionSettings ps;
	const auto& pdps = pd["partitionSettings"];
	ps.banks = pdps["banks"];
	ps.islands = pdps["islands"];
	ps.regularityDelay = pdps["regularityDelay"];
	ps.intersectionDelay = pdps["intersectionDelay"];
	ps.admissibleRadiusFactor = pdps["admissibleRadiusFactor"];

	simplesets::ComputeDrawingSettings cds;
	const auto& pdcds = pd["computeDrawingSettings"];
	cds.smooth = pdcds["smooth"];
	cds.cutoutRadiusFactor = pdcds["cutoutRadiusFactor"];
	cds.smoothingRadiusFactor = pdcds["smoothingRadiusFactor"];

	double cover = pd["cover"];

	// Partition
	auto partitions = simplesets::partition(points, gs, ps, 8 * CGAL::to_double(gs.dilationRadius()));

	// Draw
	simplesets::Partition* thePartition;
	bool found = false;
	for (auto& [time, partition] : partitions) {
		if (time < cover * gs.dilationRadius()) {
			thePartition = &partition;
			found = true;
		}
	}
	simplesets::Partition& partition = found ? (*thePartition) : partitions.front().second;

	bool wellSeparated = true;
	for (const auto& p : points) {
		for (const auto& q : points) {
			if (p.category == q.category) continue;
			if (CGAL::squared_distance(p.point, q.point) < 4 * gs.pointSize * gs.pointSize) {
				wellSeparated = false;
			}
		}
	}
	if (wellSeparated) {
		auto dpd = simplesets::DilatedPatternDrawing(partition, gs, cds);
		auto ssPainting = std::make_shared<simplesets::SimpleSetsPainting>(dpd, ds);
		SvgRenderer svgRenderer;
		svgRenderer.addPainting(ssPainting);
		svgRenderer.save(outputFilename);
	} else {
		std::cerr << "Points of different category are too close together; not computing a drawing." << std::endl;
	}
}

EMSCRIPTEN_BINDINGS(map_generators) {
	function("createSvg", optional_override([](std::string pf, std::string of) { return createSvg(pf, of); }));
}
