#include "boundary_map_reader.h"

#include <ipedoc.h>
#include <ipepath.h>

#include <CGAL/enum.h>
#include <stdexcept>

#include "cartocrow/reader/ipe_reader.h"

namespace cartocrow {

BoundaryMap ipeToBoundaryMap(const std::filesystem::path& file) {
	BoundaryMap map = BoundaryMap();

	std::shared_ptr<ipe::Document> document = IpeReader::loadIpeFile(file);

	if (document->countPages() == 0) {
		throw std::runtime_error("Cannot read map from an Ipe file with no pages");
	} else if (document->countPages() > 1) {
		throw std::runtime_error("Cannot read map from an Ipe file with more than one page");
	}

	ipe::Page* page = document->page(0);

	for (int i = 0; i < page->count(); ++i) {
		ipe::Object* object = page->object(i);
		int layer = page->layerOf(i);
		ipe::Object::Type type = object->type();
		if (type != ipe::Object::Type::EPath) {
			continue;
		}
		ipe::Path* path = object->asPath();
		ipe::Matrix matrix = path->matrix();
		ipe::Shape shape = path->shape();

		for (int i = 0; i < shape.countSubPaths(); ++i) {
			map.boundaries.push_back(Boundary());
			Boundary& b = map.boundaries.back();
			if (shape.subPath(i)->type() != ipe::SubPath::ECurve) {
				throw std::runtime_error("Encountered shape with a non-polygonal boundary");
			}
			// read the curve
			const ipe::Curve* curve = shape.subPath(i)->asCurve();
			// NB: does not include closing segment
			for (int j = 0; j < curve->countSegments(); ++j) {
				ipe::CurveSegment segment = curve->segment(j);
				if (segment.type() != ipe::CurveSegment::ESegment) {
					throw std::runtime_error("Encountered shape with a non-polygonal boundary");
				}
				if (j == 0) {
					ipe::Vector v = matrix * segment.cp(0);
					b.points.push_back(Point<Exact>(v.x, v.y));
				}
				ipe::Vector v = matrix * segment.last();
				Point<Exact> p(v.x, v.y);
				if (p != b.points.back()) {
					b.points.push_back(Point<Exact>(v.x, v.y));
				}
			}
			b.closed = curve->closed();
			if (b.closed && b.points.front() == b.points.back()) {
				// redundant point duplication in the ipe file, let's trim it
				b.points.pop_back();
			}
		}

	}

	return map;
}

} // namespace cartocrow
