#include "parse_points.h"

#include <ipereference.h>
#include <ipepath.h>

namespace cartocrow::chorematic_map {
std::vector<WeightedPoint> readPointsFromIpePage(ipe::Page* page, ipe::Cascade* cascade) {
	std::vector<WeightedPoint> points;
	for (int i = 0; i < page->count(); i++) {
		auto object = page->object(i);
		if (object->type() != ipe::Object::Type::EReference)
			continue;
		auto reference = object->asReference();
		auto matrix = object->matrix();
		ipe::Color color;
		if (reference->flags() & ipe::Reference::EHasFill) {
			auto fill = reference->fill();
			color = cascade->find(ipe::Kind::EColor, fill).color();
		} else {
			auto stroke = reference->stroke();
			color = cascade->find(ipe::Kind::EColor, stroke).color();
		}
		double r = color.iRed.toDouble();
		double b = color.iBlue.toDouble();
		auto pos = matrix * reference->position();
		auto weight = r > b ? r : -b;
		points.push_back({{pos.x, pos.y}, weight});
	}
	return points;
}

std::vector<std::vector<WeightedPoint>> readPointsFromIpe(const std::filesystem::path& path) {
	std::shared_ptr<ipe::Document> document = IpeReader::loadIpeFile(path);

	int nPages = document->countPages();
	if (nPages == 0) {
		throw std::runtime_error("Cannot read points from an Ipe file with no pages");
	}

	std::vector<std::vector<WeightedPoint>> result;
	for (int i = 0; i < nPages; ++i) {
		ipe::Page* page = document->page(i);
		result.push_back(readPointsFromIpePage(page, document->cascade()));
	}

	return result;
}

std::vector<InducedDisk> readDisksFromIpe(const std::filesystem::path& path) {
	std::shared_ptr<ipe::Document> document = IpeReader::loadIpeFile(path);

	int nPages = document->countPages();
	if (nPages == 0) {
		throw std::runtime_error("Cannot read points from an Ipe file with no pages");
	}

	std::vector<std::tuple<std::optional<Point<Inexact>>, std::optional<Point<Inexact>>, std::optional<Point<Inexact>>>> result;
	for (int i = 0; i < nPages; ++i) {
		ipe::Page* page = document->page(i);
		result.push_back(readDiskFromIpePage(page));
	}

	return result;
}

InducedDisk readDiskFromIpePage(ipe::Page* page) {
	std::optional<Point<Inexact>> p1;
	std::optional<Point<Inexact>> p2;
	std::optional<Point<Inexact>> p3;

	for (int i = 0; i < page->count(); i++) {
		auto object = page->object(i);
		if (object->type() != ipe::Object::Type::EPath)
			continue;
		ipe::Path* path = object->asPath();
		auto matrix = object->matrix();
		auto curve = path->shape().subPath(0)->asCurve();
		std::vector<Point<Inexact>> points;

		for (int k = 0; k < curve->countSegments(); k++) {
			auto segment = curve->segment(k);
			auto start = matrix * segment.cp(0);
			points.emplace_back(start.x, start.y);
		}

		auto last = matrix * curve->segment(curve->countSegments()-1).last();
		points.emplace_back(last.x, last.y);
		int s = points.size();
		if (s > 1 && points.front() == points.back()) {
			--s;
		}
		if (s > 0) {
			p1 = points[0];
		}
		if (s > 1) {
			p2 = points[1];
		}
		if (s > 2) {
			p3 = points[2];
		}

		break;
	}

	return {p1, p2, p3};
}
}