/*
The Necklace Map console application implements the algorithmic
geo-visualization method by the same name, developed by
Bettina Speckmann and Kevin Verbeek at TU Eindhoven
(DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 10-09-2019
*/

#include "cartocrow/necklace_map/painting.h"
#include "cartocrow/renderer/ipe_renderer.h"
#include <fstream>
#include <sstream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <cartocrow/core/timer.h>
#include <cartocrow/necklace_map/necklace_map.h>

#include <console/common/utils_cla.h>
#include <console/common/utils_flags.h>

DEFINE_string(map_file, "", "Input map, in Ipe format.");

DEFINE_string(data_file, "", "Input data file.");

DEFINE_string(data_field, "", "Column name from the data file to take the data values from.");

DEFINE_string(output, "", "The file to which to write the output.");

DEFINE_string(
    interval_type, "wedge",
    "The interval type used to map regions onto feasible intervals (\"centroid\" or \"wedge\").");

DEFINE_double(centroid_interval_length, 0.2 * M_PI,
              "The arc length of centroid intervals in radians (only used with "
              "--interval_type=centroid). Must be in the range [0, pi].");

DEFINE_double(wedge_interval_min_length, 0,
              "The minimum arc length of wedge intervals in radians (only used with "
              "--interval_type=wedge). Must be in the range [0, pi].");

DEFINE_string(order_type, "any",
              "The order type enforced by the scale factor algorithm (\"fixed\" or \"any\").");

DEFINE_int32(search_depth, 10, "The search depth used during binary searches on the decision space.");

DEFINE_int32(heuristic_cycles, 5,
             "The number of heuristic iterations used by the bead order search (only used with "
             "--order_type=any). If 0, the exact algorithm is used.");

DEFINE_double(buffer, 0,
              "Minimum distance between the necklace beads in radians. Must be in range [0, pi].");

DEFINE_int32(placement_iterations, 30, "The number of iterations used by the placement heuristic.");

DEFINE_double(aversion_ratio, 0.001,
              "Measure for repulsion between necklace beads as opposed by the attraction to the "
              "feasible interval center. Must be in the range (0, 1].");

DEFINE_double(bead_opacity, 1, "Opacity with which to draw the beads. Must be in the range [0, 1].");

DEFINE_bool(draw_necklace_curve, true, "Whether to draw the necklace shape in the output.");

DEFINE_bool(draw_necklace_kernel, false, "Whether to draw the necklace kernel in the output.");

DEFINE_bool(draw_feasible_intervals, false, "Whether to draw the feasible intervals in the output.");

DEFINE_bool(draw_valid_intervals, false, "Whether to draw the valid intervals in the output.");

DEFINE_bool(draw_connectors, false,
            "Whether to draw a line from each region centroid to its "
            "bead center.");

void validateFlags(cartocrow::necklace_map::Parameters& parameters,
                   cartocrow::necklace_map::Painting::Options& painting_options) {
	bool correct = true;
	LOG(INFO) << "necklace_map_cla flags:";

	// Note that we mainly print flags to enable reproducibility.
	// Other flags are validated, but only printed if not valid.
	// Note that we may skip some low-level flags that almost never change.
	using namespace validate;

	// There must be input geometry and input numeric data.
	correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(map_file), ExistsFile);
	correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(data_file), ExistsFile);
	correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(data_field), Not<Empty>);

	// Note that we allow overwriting existing output.
	correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(output), Or<Empty, MakeAvailableFile>);

	// Interval parameters.
	{
		correct &=
		    CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(interval_type),
		                      cartocrow::necklace_map::IntervalTypeParser(parameters.interval_type));

		correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(centroid_interval_length),
		                             MakeRangeCheck(0.0, M_PI));
		parameters.centroid_interval_length_rad = FLAGS_centroid_interval_length;

		correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(wedge_interval_min_length),
		                             MakeRangeCheck(0.0, M_PI));
		parameters.wedge_interval_length_min_rad = FLAGS_wedge_interval_min_length;
	}

	// Scale factor optimization parameters.
	{
		correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(order_type),
		                             cartocrow::necklace_map::OrderTypeParser(parameters.order_type));

		correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(buffer), MakeRangeCheck(0.0, M_PI));
		parameters.buffer_rad = FLAGS_buffer;

		correct &=
		    CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(search_depth), MakeStrictLowerBoundCheck(0));
		parameters.binary_search_depth = FLAGS_search_depth;

		correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(heuristic_cycles), MakeLowerBoundCheck(0));
		parameters.heuristic_cycles = FLAGS_heuristic_cycles;
	}

	// Placement parameters.
	{
		correct &=
		    CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(placement_iterations), MakeLowerBoundCheck(0));
		parameters.placement_cycles = FLAGS_placement_iterations;

		correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(aversion_ratio),
		                             MakeRangeCheck<Closure::kClosed, Closure::kClosed>(0.0, 1.0));
		parameters.aversion_ratio = FLAGS_aversion_ratio;
	}

	// Output parameters.
	using Options = cartocrow::necklace_map::Painting::Options;
	painting_options = Options();
	{
		correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(bead_opacity), MakeRangeCheck(0.0, 1.0));
		painting_options.m_beadOpacity = FLAGS_bead_opacity;

		PrintFlag(FLAGS_NAME_AND_VALUE(draw_necklace_curve));
		painting_options.m_drawNecklaceCurve = FLAGS_draw_necklace_curve;

		PrintFlag(FLAGS_NAME_AND_VALUE(draw_necklace_kernel));
		painting_options.m_drawNecklaceKernel = FLAGS_draw_necklace_kernel;

		PrintFlag(FLAGS_NAME_AND_VALUE(draw_connectors));
		painting_options.m_drawConnectors = FLAGS_draw_connectors;
	}

	correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(log_dir), Or<Empty, IsDirectory>);
	PrintFlag(FLAGS_NAME_AND_VALUE(stderrthreshold));
	PrintFlag(FLAGS_NAME_AND_VALUE(v));

	if (!correct) {
		LOG(FATAL) << "Encountered invalid command-line options";
	}
}

void writeOutput(const std::vector<cartocrow::necklace_map::NecklaceMapElement::Ptr>& elements,
                 const std::vector<cartocrow::necklace_map::Necklace::Ptr>& necklaces,
                 const cartocrow::Number& scale_factor,
                 const cartocrow::necklace_map::Painting::Options& options) {
	cartocrow::necklace_map::Painting painting(elements, necklaces, scale_factor, options);
	cartocrow::renderer::IpeRenderer renderer(painting);
	renderer.save(FLAGS_output);
}

int main(int argc, char** argv) {
	InitApplication(argc, argv, "Computes a necklace map from a given input map and data values.",
	                {"--map_file=<file>", "--data_file=<file>", "--data_field=<column>"});

	cartocrow::necklace_map::Parameters parameters;
	cartocrow::necklace_map::Painting::Options options;
	validateFlags(parameters, options);

	cartocrow::Timer time;

	using MapElement = cartocrow::necklace_map::NecklaceMapElement;
	using Necklace = cartocrow::necklace_map::Necklace;
	std::vector<MapElement::Ptr> elements;
	std::vector<Necklace::Ptr> necklaces;

	cartocrow::necklace_map::IpeReader map_reader;
	const bool success_read_map = map_reader.readFile(FLAGS_map_file, elements, necklaces);
	cartocrow::necklace_map::DataReader data_reader;
	const bool success_read_data = data_reader.ReadFile(FLAGS_data_file, FLAGS_data_field, elements);
	CHECK(success_read_map && success_read_data) << "Terminating program.";
	const double time_read = time.Stamp();

	cartocrow::Number scaleFactor =
	    cartocrow::necklace_map::ComputeScaleFactor(parameters, elements, necklaces);
	LOG(INFO) << "Computed scale factor: " << scaleFactor;
	const double time_compute = time.Stamp();

	writeOutput(elements, necklaces, scaleFactor, options);
	const double time_write = time.Stamp();

	const double time_total = time.Span();

	LOG(INFO) << "Time cost (read files): " << time_read;
	LOG(INFO) << "Time cost (compute NM): " << time_compute;
	LOG(INFO) << "Time cost (serialize):  " << time_write;
	LOG(INFO) << "Time cost (total):      " << time_total;

	return 0;
}
