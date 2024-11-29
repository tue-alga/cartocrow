#include "chorematic_map_demo.h"
#include <QApplication>
#include <utility>

#include <CGAL/mark_domain_in_triangulation.h>

std::vector<WeightedPoint> readPointsFromIpePage(ipe::Page* page) {
	std::vector<WeightedPoint> points;
	for (int i = 0; i < page->count(); i++) {
		auto object = page->object(i);
		if (object->type() != ipe::Object::Type::EReference) continue;
		auto reference = object->asReference();
		auto matrix = object->matrix();
		std::string colorName = reference->fill().string().z();
		auto pos = matrix * reference->position();
		auto weight = colorName == "CB light blue" ? -1.0 : 1.0;
		points.push_back({{pos.x, pos.y}, weight});
	}
	return points;
}

std::vector<WeightedPoint> readPointsFromIpe(const std::filesystem::path& path) {
	std::shared_ptr<ipe::Document> document = IpeReader::loadIpeFile(path);

	if (document->countPages() == 0) {
		throw std::runtime_error("Cannot read points from an Ipe file with no pages");
	} else if (document->countPages() > 1) {
		throw std::runtime_error("Cannot read points from an Ipe file with more than one page");
	}

	ipe::Page* page = document->page(0);
	return readPointsFromIpePage(page);
}

class RegionArrangementPainting : public GeometryPainting {
  private:
	std::shared_ptr<RegionArrangement> m_arr;
  public:
	RegionArrangementPainting(std::shared_ptr<RegionArrangement> arr) : m_arr(std::move(arr)) {};

	void paint(GeometryRenderer &renderer) const override {
		renderer.setMode(GeometryRenderer::stroke);
		renderer.setStroke(Color{0, 0, 0}, 1.0);
		for (auto eit = m_arr->edges_begin(); eit != m_arr->edges_end(); ++eit) {
			renderer.draw(Segment<Exact>(eit->source()->point(), eit->target()->point()));
		}
	}
};

class TriangulationPainting : public GeometryPainting {
  private:
	std::shared_ptr<CDT> m_cdt;
	std::unordered_map<CDT::Face_handle, bool> m_in_domain;

  public:
	TriangulationPainting(std::shared_ptr<CDT> cdt, std::unordered_map<CDT::Face_handle, bool> in_domain) : m_cdt(std::move(cdt)), m_in_domain(std::move(in_domain)) {};

	void paint(GeometryRenderer &renderer) const override {
		renderer.setMode(GeometryRenderer::stroke);
		renderer.setStroke(Color{0, 0, 0}, 1.0);
		for (auto eit = m_cdt->edges_begin(); eit != m_cdt->edges_end(); ++eit) {
			if (!m_in_domain.at(eit->first)) continue;
			auto p1 = eit->first->vertex((eit->second + 1) % 3)->point();
			auto p2 = eit->first->vertex((eit->second + 2) % 3)->point();
			renderer.draw(Segment<Exact>(p1, p2));
		}
	}
};

ChorematicMapDemo::ChorematicMapDemo() {
	setWindowTitle("Chorematic map");
	auto renderer = new GeometryWidget();
	renderer->setDrawAxes(false);
	setCentralWidget(renderer);

	auto points = readPointsFromIpe("points.ipe");
	auto [p1, p2, p3] = maximum_weight_disk(points.begin(), points.end());

	Circle<Inexact> circle(p1.point, p2.point, p3.point);

	renderer->addPainting([points](renderer::GeometryRenderer& renderer) {
		for (const auto& [point, weight] : points) {
			if (weight > 0) {
				renderer.setFill(0xFF0000);
				renderer.setStroke(0xFF0000, 1.0);
				renderer.draw(point);
			}
			if (weight < 0) {
				renderer.setFill(0x0000FF);
				renderer.setStroke(0x0000FF, 1.0);
				renderer.draw(point);
			}
		}
	}, "Points");

	renderer->addPainting([circle](renderer::GeometryRenderer& renderer) {
		renderer.setMode(GeometryRenderer::fill | GeometryRenderer::stroke);
	  	renderer.setStroke(Color(0, 0, 0), 2.0);
	  	renderer.setFill(0x000000);
		renderer.setFillOpacity(50);
	  	renderer.draw(circle);
	}, "Circle");

	auto regionMap = ipeToRegionMap("data/test_region_arrangement.ipe");
	m_arr = std::make_shared<RegionArrangement>(regionMapToArrangement(regionMap));
	m_cdt = std::make_shared<CDT>();

	for (auto fit = m_arr->faces_begin(); fit != m_arr->faces_end(); ++fit) {
		std::vector<RegionArrangement::Ccb_halfedge_circulator> ccbs;
		std::copy(fit->outer_ccbs_begin(), fit->outer_ccbs_end(), std::back_inserter(ccbs));
		std::copy(fit->inner_ccbs_begin(), fit->inner_ccbs_end(), std::back_inserter(ccbs));
		for (auto ccb : ccbs) {
			std::vector<Point<Exact>> vertices;
			auto curr = ccb;
			do {
				auto point = curr->source()->point();
				vertices.push_back(point);
			} while (++curr != ccb);
			m_cdt->insert_constraint(vertices.begin(), vertices.end(), true);
		}
	}

	std::unordered_map<CDT::Face_handle, bool> in_domain_map;
	auto in_domain = std::make_shared<boost::associative_property_map<std::unordered_map<CDT::Face_handle,bool>>>(in_domain_map);

	if (m_cdt->dimension() == 2) {
		CGAL::mark_domain_in_triangulation(*m_cdt, *in_domain);
	}

	for (auto fh : m_cdt->finite_face_handles()) {
		in_domain_map.contains(fh);
	}
	auto rap = std::make_shared<RegionArrangementPainting>(m_arr);
	renderer->addPainting(rap, "Region arrangement");
	auto tp = std::make_shared<TriangulationPainting>(m_cdt, in_domain_map);
	renderer->addPainting(tp, "Triangulation");
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	ChorematicMapDemo demo;
	demo.show();
	app.exec();
}
