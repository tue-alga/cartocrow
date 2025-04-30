#include "../catch.hpp"

#include "cartocrow/reader/ipe_reader.h"
#include "cartocrow/renderer/geometry_painting.h"
#include "cartocrow/renderer/ipe_renderer.h"

#include <ipeattributes.h>
#include <ipedoc.h>
#include <ipeshape.h>

#include <filesystem>

using namespace cartocrow;

TEST_CASE("Exporting marks to Ipe") {
	class TestPainting : public renderer::GeometryPainting {
		void paint(renderer::GeometryRenderer& renderer) const override {
			renderer.draw(Point<Exact>(0, 0));
			renderer.draw(Point<Exact>(2, 1));
		}
	};
	auto painting = std::make_shared<TestPainting>();
	renderer::IpeRenderer renderer(painting);
	std::filesystem::path path = std::filesystem::temp_directory_path() / "test.ipe";
	renderer.save(path);

	std::shared_ptr<ipe::Document> result = IpeReader::loadIpeFile(path);
	REQUIRE(result->countPages() == 1);
	REQUIRE(result->page(0)->count() == 2);
	REQUIRE(result->page(0)->object(0)->type() == ipe::Object::Type::EReference);
	CHECK(result->page(0)->object(0)->asReference()->position() == ipe::Vector(0, 0));
	REQUIRE(result->page(0)->object(1)->type() == ipe::Object::Type::EReference);
	CHECK(result->page(0)->object(1)->asReference()->position() == ipe::Vector(2, 1));
}

TEST_CASE("Exporting a line segment to Ipe") {
	class TestPainting : public renderer::GeometryPainting {
		void paint(renderer::GeometryRenderer& renderer) const override {
			renderer.draw(Segment<Exact>(Point<Exact>(2, 3), Point<Exact>(1, 4)));
		}
	};
	auto painting = std::make_shared<TestPainting>();
	renderer::IpeRenderer renderer(painting);
	std::filesystem::path path = std::filesystem::temp_directory_path() / "test.ipe";
	renderer.save(path);

	std::shared_ptr<ipe::Document> result = IpeReader::loadIpeFile(path);
	REQUIRE(result->countPages() == 1);
	REQUIRE(result->page(0)->count() == 1);
	REQUIRE(result->page(0)->object(0)->type() == ipe::Object::Type::EPath);
	const ipe::Path* ipePath = result->page(0)->object(0)->asPath();
	REQUIRE(ipePath->shape().countSubPaths() == 1);
	REQUIRE(ipePath->shape().subPath(0)->type() == ipe::SubPath::Type::ECurve);
	const ipe::Curve* ipeCurve = ipePath->shape().subPath(0)->asCurve();
	REQUIRE(ipeCurve->countSegments() == 1);
	REQUIRE(ipeCurve->segment(0).type() == ipe::CurveSegment::Type::ESegment);
	CHECK(ipeCurve->segment(0).cp(0) == ipe::Vector(2, 3));
	CHECK(ipeCurve->segment(0).cp(1) == ipe::Vector(1, 4));
}

TEST_CASE("Exporting a label to Ipe") {
	std::shared_ptr<renderer::GeometryPainting> painting;
	std::string expectedText = "";
	SECTION("without special characters") {
		class TestPainting : public renderer::GeometryPainting {
			void paint(renderer::GeometryRenderer& renderer) const override {
				renderer.drawText(Point<Exact>(5, 5), "Hello!");
			}
		};
		painting = std::make_shared<TestPainting>();
		expectedText = "Hello!";
	}
	SECTION("with special characters") {
		class TestPainting : public renderer::GeometryPainting {
			void paint(renderer::GeometryRenderer& renderer) const override {
				renderer.drawText(Point<Exact>(5, 5), "test # $ % & { } _ ~ ^ \\ test");
			}
		};
		painting = std::make_shared<TestPainting>();
		expectedText = "test \\# \\$ \\% \\& \\{ \\} \\_ \\~{} \\^{} \\textbackslash{} test";
	}
	renderer::IpeRenderer renderer(painting);
	std::filesystem::path path = std::filesystem::temp_directory_path() / "test.ipe";
	renderer.save(path);

	std::shared_ptr<ipe::Document> result = IpeReader::loadIpeFile(path);
	REQUIRE(result->countPages() == 1);
	REQUIRE(result->page(0)->count() == 1);
	REQUIRE(result->page(0)->object(0)->type() == ipe::Object::Type::EText);
	const ipe::Text* ipeText = result->page(0)->object(0)->asText();
	REQUIRE(ipeText->textType() == ipe::Text::TextType::ELabel);
	REQUIRE(ipeText->horizontalAlignment() == ipe::THorizontalAlignment::EAlignHCenter);
	REQUIRE(ipeText->verticalAlignment() == ipe::TVerticalAlignment::EAlignVCenter);
	REQUIRE(ipeText->text().z() == expectedText);
}
