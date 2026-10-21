#include "../catch.hpp"

#include "cartocrow/core/core.h"
#include "cartocrow/core/halfplane.h"
#include "cartocrow/core/polyline.h"

using namespace cartocrow;

TEST_CASE("Number conversions") {
	Number<Exact> ex1, ex2;
	Number<Inexact> in1, in2;

	// conversions
	in1 = approximate(ex1);
	ex1 = pretendExact(in1);

	// same kernels
	in2 = approximate(in1);
	ex2 = pretendExact(ex1);

	// raw convert with autoinference
	auto a = convert_kernel<Exact>(ex1);
	auto b = convert_kernel<Exact>(in1);
	auto c = convert_kernel<Inexact>(ex1);
	auto d = convert_kernel<Inexact>(in1);
}

TEST_CASE("Point conversions") {
	Point<Exact> ex1, ex2;
	Point<Inexact> in1, in2;

	// conversions
	in1 = approximate(ex1);
	ex1 = pretendExact(in1);

	// same kernels
	in2 = approximate(in1);
	ex2 = pretendExact(ex1);

	// raw convert with autoinference
	auto a = convert_kernel<Exact>(ex1);
	auto b = convert_kernel<Exact>(in1);
	auto c = convert_kernel<Inexact>(ex1);
	auto d = convert_kernel<Inexact>(in1);
}

TEST_CASE("Polygon conversions") {
	Polygon<Exact> ex1, ex2;
	Polygon<Inexact> in1, in2;

	// conversions
	in1 = approximate(ex1);
	ex1 = pretendExact(in1);

	// same kernels
	in2 = approximate(in1);
	ex2 = pretendExact(ex1);

	// raw convert with autoinference
	auto a = convert_kernel<Exact>(ex1);
	auto b = convert_kernel<Exact>(in1);
	auto c = convert_kernel<Inexact>(ex1);
	auto d = convert_kernel<Inexact>(in1);
}

TEST_CASE("Polyline conversions") {
	Polyline<Exact> ex1, ex2;
	Polyline<Inexact> in1, in2;

	// conversions
	in1 = approximate(ex1);
	ex1 = pretendExact(in1);

	// same kernels
	in2 = approximate(in1);
	ex2 = pretendExact(ex1);

	// raw convert with autoinference
	auto a = convert_kernel<Exact>(ex1);
	auto b = convert_kernel<Exact>(in1);
	auto c = convert_kernel<Inexact>(ex1);
	auto d = convert_kernel<Inexact>(in1);
}

TEST_CASE("Halfplane conversions") {
	Halfplane<Exact> ex1 = Halfplane(Line<Exact>()), ex2 = Halfplane(Line<Exact>());
	Halfplane<Inexact> in1 = Halfplane(Line<Inexact>()), in2 = Halfplane(Line<Inexact>());

	// conversions
	in1 = approximate(ex1);
	ex1 = pretendExact(in1);

	// same kernels
	in2 = approximate(in1);
	ex2 = pretendExact(ex1);

	// raw convert with autoinference
	auto a = convert_kernel<Exact>(ex1);
	auto b = convert_kernel<Exact>(in1);
	auto c = convert_kernel<Inexact>(ex1);
	auto d = convert_kernel<Inexact>(in1);
}