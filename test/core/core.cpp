#include "../catch.hpp"

#include <CGAL/Arr_walk_along_line_point_location.h>
#include <CGAL/enum.h>
#include <ctime>

#include "../../cartocrow/core/core.h"
#include "../../cartocrow/core/timer.h"

using namespace cartocrow;

// The test cases in this file mostly exercise CGAL functions and are mostly
// intended to test that CartoCrow's aliases work correctly, and to provide some
// simple usage examples.
// Hence, we don't comprehensively test the CGAL functionality here.

TEST_CASE("Creating and comparing numbers") {
	Number<Exact> exactZero(0);
	Number<Inexact> inexactZero(0);
	CHECK(exactZero == exactZero);
	CHECK(exactZero == 0.0);
	CHECK(inexactZero == inexactZero);
	CHECK(inexactZero == 0.0);
	Number<Exact> exactOneThird = Number<Exact>(1) / Number<Exact>(3);
	Number<Inexact> inexactOneThird = Number<Inexact>(1) / Number<Inexact>(3);
	CHECK(exactOneThird == exactOneThird);
	CHECK(exactOneThird != 1.0 / 3.0); // cannot represent 1/3 as a double
	CHECK(inexactOneThird == inexactOneThird);
	CHECK(inexactOneThird == 1.0 / 3.0);
}

TEST_CASE("Creating some basic geometry") {
	// make point at (2, 0)
	Point<Exact> p1(2, 0);
	Point<Inexact> p2(2, 0);

	// make vector (1, 1)
	Vector<Exact> v1(1, 1);
	Vector<Inexact> v2(1, 1);

	// make another point by adding the vector to the point
	Point<Exact> q1 = p1 + v1;
	CHECK(q1.x() == 3);
	CHECK(q1.y() == 1);
	Point<Inexact> q2 = p2 + v2;
	CHECK(q2.x() == Approx(3));
	CHECK(q2.y() == Approx(1));
}

TEST_CASE("Creating circles") {
	// make circle around the origin with radius sqrt(25) = 5
	Circle<Exact> c1(CGAL::ORIGIN, 25);
	CHECK(c1.has_on_boundary(Point<Exact>(5, 0)));
	CHECK(c1.has_on_boundary(Point<Exact>(3, 4)));
	CHECK(c1.has_on_bounded_side(Point<Exact>(4, 0)));
	CHECK(c1.has_on_unbounded_side(Point<Exact>(6, 0)));

	// make circle passing through (0, 0), (1, 0), and (0, 1)
	Circle<Exact> c2(CGAL::ORIGIN, Point<Exact>(1, 0), Point<Exact>(0, 1));
	CHECK(c2.has_on_bounded_side(Point<Exact>(0, 0.5)));
	CHECK(c2.has_on_bounded_side(Point<Exact>(0.5, 0)));
	Number<Exact> epsilon(1 / Number<Exact>(1e100));
	CHECK(c2.has_on_bounded_side(Point<Exact>(1 - epsilon, 1)));
	CHECK(c2.has_on_boundary(Point<Exact>(1, 1)));
	CHECK(c2.has_on_unbounded_side(Point<Exact>(1 + epsilon, 1)));
	CHECK(c2.has_on_bounded_side(Point<Exact>(1, 1 - epsilon)));
	CHECK(c2.has_on_boundary(Point<Exact>(1, 1)));
	CHECK(c2.has_on_unbounded_side(Point<Exact>(1, 1 + epsilon)));

	// check for circle equality
	CHECK(c2 == Circle<Exact>(Point<Exact>(0.5, 0.5), 0.5));
}

TEST_CASE("Creating lines") {
	// make line through (1, 0) in the direction of vector (1, 1)
	Line<Exact> l1(Point<Exact>(1, 0), Vector<Exact>(1, 1));
	CHECK(l1.has_on(Point<Exact>(1.5, 0.5)));
	CHECK(l1.has_on_positive_side(Point<Exact>(1.5, 1)));
	CHECK(l1.has_on_negative_side(Point<Exact>(1.5, 0)));
	CHECK(l1.projection(Point<Exact>(2, 0)) == Point<Exact>(1.5, 0.5));

	// check for line equality
	Line<Exact> l2(Point<Exact>(0, -1), Point<Exact>(3, 2));
	CHECK(l1 == l2);

	// lines in the opposite direction are not equal
	Line<Exact> l3(Point<Exact>(1, 0), Vector<Exact>(-1, -1));
	CHECK(l1 != l3);
}

TEST_CASE("Creating segments") {
	// make segment from (1, 0) to (2, 2)
	Segment<Exact> s1(Point<Exact>(1, 0), Point<Exact>(2, 2));
	CHECK(s1.has_on(Point<Exact>(1.5, 1)));
	CHECK(!s1.has_on(Point<Exact>(0, -2)));
	CHECK(s1.supporting_line().has_on(Point<Exact>(0, -2)));
	CHECK(!s1.has_on(Point<Exact>(3, 4)));
	CHECK(s1.supporting_line().has_on(Point<Exact>(3, 4)));
}

TEST_CASE("Creating polygons") {
	// make polygon with vertices (0, 0), (1, 0), (0, 1)
	Polygon<Exact> p1;
	p1.push_back(CGAL::ORIGIN);
	p1.push_back(Point<Exact>(1, 0));
	p1.push_back(Point<Exact>(0, 1));
	CHECK(p1.is_simple());
	CHECK(p1.is_convex());
	CHECK(p1.area() == 0.5);
	CHECK(p1.orientation() == CGAL::COUNTERCLOCKWISE);
	CHECK(p1.has_on_boundary(Point<Exact>(0.5, 0.5)));
	CHECK(p1.has_on_bounded_side(Point<Exact>(0.25, 0.25)));
	CHECK(p1.has_on_unbounded_side(Point<Exact>(1, 1)));

	// make a non-simple polygon
	Polygon<Exact> p2;
	p2.push_back(CGAL::ORIGIN);
	p2.push_back(Point<Exact>(1, 0));
	p2.push_back(Point<Exact>(0, 1));
	p2.push_back(Point<Exact>(1, 1));
	CHECK(!p2.is_simple());
	CHECK(!p2.is_convex());
	CHECK(p2.area() == 0);
}

TEST_CASE("Creating polygons with holes") {
	// make polygon with one hole
	Polygon<Exact> outside;
	outside.push_back(CGAL::ORIGIN);
	outside.push_back(Point<Exact>(1, 0));
	outside.push_back(Point<Exact>(0, 1));
	Polygon<Exact> hole;
	hole.push_back(Point<Exact>(0.25, 0.25));
	hole.push_back(Point<Exact>(0.75, 0.25));
	hole.push_back(Point<Exact>(0.25, 0.75));
	PolygonWithHoles<Exact> p1(outside);
	CHECK(p1.number_of_holes() == 0);
	p1.add_hole(hole);
	CHECK(p1.number_of_holes() == 1);
}

TEST_CASE("Map") {
	Map<Exact> map;
	CHECK(map.number_of_vertices() == 0);
	CHECK(map.number_of_edges() == 0);
	CHECK(map.number_of_faces() == 1);

	// manually inserting elements
	Map<Exact>::Vertex_handle v1 =
	    map.insert_in_face_interior(Point<Exact>(0, 0), map.unbounded_face());
	CHECK(map.number_of_vertices() == 1);
	CHECK(map.number_of_edges() == 0);
	CHECK(map.number_of_faces() == 1);

	Map<Exact>::Vertex_handle v2 =
	    map.insert_in_face_interior(Point<Exact>(1, 0), map.unbounded_face());
	CHECK(map.number_of_vertices() == 2);
	CHECK(map.number_of_edges() == 0);
	CHECK(map.number_of_faces() == 1);

	map.insert_at_vertices(Segment<Exact>(Point<Exact>(0, 0), Point<Exact>(1, 0)), v1, v2);
	CHECK(map.number_of_vertices() == 2);
	CHECK(map.number_of_edges() == 1);
	CHECK(map.number_of_faces() == 1);

	// free functions (allow inserting arbitrary segments, even crossing ones)
	CGAL::insert<>(map, Segment<Exact>(Point<Exact>(0, -1), Point<Exact>(1, 1)));
	CHECK(map.number_of_vertices() == 5);
	CHECK(map.number_of_edges() == 4);
	CHECK(map.number_of_faces() == 1);

	CGAL::insert<>(map, Segment<Exact>(Point<Exact>(1, 0), Point<Exact>(1, 1)));
	CHECK(map.number_of_vertices() == 5);
	CHECK(map.number_of_edges() == 5);
	CHECK(map.number_of_faces() == 2);

	CGAL::insert<>(map, Segment<Exact>(Point<Exact>(0, 0.5), Point<Exact>(1, 0)));
	CHECK(map.number_of_vertices() == 7);
	CHECK(map.number_of_edges() == 8);
	CHECK(map.number_of_faces() == 3);

	// point location queries
	CGAL::Arr_walk_along_line_point_location<Map<Exact>> locator(map);
	auto l1 = locator.locate(Point<Exact>(0.6, 0.1));
	REQUIRE(l1.type() == typeid(Map<Exact>::Face_const_handle));
	auto f1 = boost::get<Map<Exact>::Face_const_handle>(l1);
	auto l2 = locator.locate(Point<Exact>(0.7, 0.1));
	REQUIRE(l2.type() == typeid(Map<Exact>::Face_const_handle));
	auto f2 = boost::get<Map<Exact>::Face_const_handle>(l2);
	auto l3 = locator.locate(Point<Exact>(0.9, 0.5));
	REQUIRE(l3.type() == typeid(Map<Exact>::Face_const_handle));
	auto f3 = boost::get<Map<Exact>::Face_const_handle>(l3);
	auto l4 = locator.locate(Point<Exact>(0, 1));
	REQUIRE(l4.type() == typeid(Map<Exact>::Face_const_handle));
	auto f4 = boost::get<Map<Exact>::Face_const_handle>(l4);
	CHECK(f1 == f2);
	CHECK(f1 != f3);
	CHECK(f1 != f4);
	CHECK(f2 != f4);
	CHECK(f3 != f4);
	CHECK(f4 == map.unbounded_face());

	auto l5 = locator.locate(Point<Exact>(0.25, 0));
	REQUIRE(l5.type() == typeid(Map<Exact>::Halfedge_const_handle));
	auto e5 = boost::get<Map<Exact>::Halfedge_const_handle>(l5);
	CHECK(e5->curve().line().has_on(Point<Exact>(0.125, 0)));

	auto l6 = locator.locate(Point<Exact>(0.5, 0));
	REQUIRE(l6.type() == typeid(Map<Exact>::Vertex_const_handle));
	auto v6 = boost::get<Map<Exact>::Vertex_const_handle>(l6);
	CHECK(v6->point() == Point<Exact>(0.5, 0));
}
