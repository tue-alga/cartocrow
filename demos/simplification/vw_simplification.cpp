/*
The Visvaling-Whyatt package implements the iterative algorithm for simplifying polygonal maps.
Copyright (C) 2021  TU Eindhoven

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

Created by Wouter Meulemans (w.meulemans@tue.nl) on 31-08-2021
*/

#include <fstream>
#include <sstream>

#include <gflags/gflags.h>

#include <cartocrow/core/core_types.h>
#include <cartocrow/core/timer.h>
#include <cartocrow/visvalingam-whyatt/vw_simplification.h>

#include <console/common/utils_cla.h>
#include <console/common/utils_flags.h>

// the input flags are mutually exclusive per type to prevent accidentally setting both and 'the wrong one' being used.

DEFINE_string(in_geometry_filename, "", "The input map geometry filename.");

DEFINE_string(out_filename, "",
              "The file to which to write the output, or empty if no file should be written.");

DEFINE_bool(out_website, false,
            "Whether to write the output to the standard output stream for the website.");

DEFINE_int32(target_complexity, 10, "The desired number of vertices for the output geometry");

void ValidateFlags() {
	bool correct = true;
	LOG(INFO) << "vwsimplification_cla flags:";

	// Note that we mainly print flags to enable reproducibility.
	// Other flags are validated, but only printed if not valid.
	// Note that we may skip some low-level flags that almost never change.
	using namespace validate;

	// There must be input geometry and input numeric data.
	//correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(in_geometry_filename), ExistsFile);

	// Note that we allow overwriting existing output.
	//correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(out_filename), Or<Empty, MakeAvailableFile>);

	correct &=
	    CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(target_complexity), MakeStrictLowerBoundCheck(2));

	if (!correct) {
		LOG(FATAL) << "Errors in flags; Terminating.";
	}
}

bool ReadGeometry() {
	return true; // TODO
}

void WriteOutput() {

	// TODO
}

int main(int argc, char** argv) {
	InitApplication(
	    argc, argv,
	    "Command line application that exposes the functionality of the CartoCrow necklace map.",
	    {"--in_geometry_filename=<file>"});

	ValidateFlags();

	cartocrow::Timer time;

	std::vector<cartocrow::Point> testCurve;

	cartocrow::Point a(0, 0);
	cartocrow::Point b(1, 1);
	cartocrow::Point c(2, 0);
	cartocrow::Point d(3, 2);
	cartocrow::Point e(4, -1);

	testCurve.push_back(a);
	testCurve.push_back(b);
	testCurve.push_back(c);
	testCurve.push_back(d);
	testCurve.push_back(e);

	cartocrow::visvalingam_whyatt::VWSimplification vw(&testCurve);

	const double time_read = time.Stamp();

	for (int i = 5; i >= 2; i--) {
		cartocrow::Number v = vw.ConstructAtComplexity(i);
		std::cout << "complexity " << i << "\n";
		std::cout << "max cost" << v << "\n";
		std::cout << "result"
		          << "\n";
		for (cartocrow::Point p : testCurve) {

			std::cout << p.hx() << " " << p.hy() << "\n";
		}
	}

	const double time_compute = time.Stamp();

	const double time_write = time.Stamp();

	const double time_total = time.Span();

	LOG(INFO) << "Time cost (read files): " << time_read;
	LOG(INFO) << "Time cost (compute NM): " << time_compute;
	LOG(INFO) << "Time cost (serialize):  " << time_write;
	LOG(INFO) << "Time cost (total):      " << time_total;

	return 0;
}
