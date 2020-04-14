/*
The Necklace Map console application implements the algorithmic
geo-visualization method by the same name, developed by
Bettina Speckmann and Kevin Verbeek at TU Eindhoven
(DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
Copyright (C) 2019  Netherlands eScience Center and TU Eindhoven

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

#include <fstream>
#include <sstream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <geoviz/common/timer.h>
#include <geoviz/necklace_map/necklace_map.h>

#include "console/common/utils_cla.h"
#include "console/common/utils_flags.h"
#include "console/necklace_map_io/necklace_map_io.h"


// the input flags are mutually exclusive per type to prevent accidentally setting both and 'the wrong one' being used.

DEFINE_string
(
  in_geometry_filename,
  "",
  "The input map geometry filename."
);

DEFINE_string
(
  in_data_filename,
  "",
  "The input numeric data filename."
);

DEFINE_string
(
  in_value_name,
  "",
  "The name of the data column to visualize using the necklace map."
);

DEFINE_string
(
  out_filename,
  "",
  "The file to which to write the output, or empty if no file should be written."
);

DEFINE_bool
(
  out_website,
  false,
  "Whether to write the output to the standard output stream for the website. This also forces logging to the standard error stream."
);

DEFINE_string
(
  interval_type,
  "wedge",
  "The interval type used to map regions onto feasible intervals. Must be one of {'centroid', 'wedge'}."
);

DEFINE_double
(
  centroid_interval_length_rad,
  0.2 * M_PI,
  "The arc length of centroid intervals (in radians). Must be in the range [0, pi]."
  " Note that small intervals greatly restrict the available scale factors."
);

DEFINE_bool
(
  ignore_point_regions,
  false,
  "Whether to ignore regions covering a single point on the map. If these are not ignored, their feasible interval type is set to centroid."
);

DEFINE_string
(
  order_type,
  "any",
  "The order type enforced by the scale factor algorithm. Must be one of {'fixed', 'any', 'heuristic'}."
);

DEFINE_double
(
  buffer_rad,
  0,
  "Minimum distance between the necklace beads (in radians). Must be in range [0, pi].\n"
  "Note that large values are likely to force the necklace bead area to 0."
  " Also note that values close to 0 are a lot more influential."
  " Scaling scrollbar values using a 4th degree function is recommended."
);

DEFINE_double
(
  aversion_ratio,
  0.001,
  "Measure for repulsion between necklace beads as opposed by the attraction to the feasible interval center."
  " Must be in the range (0, 1].\n"
  "Note that values close to 0 are a lot more influential."
  " Scaling scrollbar values using a 4th degree function is recommended."
);

DEFINE_int32
(
  pixel_width,
  500,
  "Output pixel width. Must be strictly positive."
);

DEFINE_int32
(
  region_coordinate_precision,
  5,
  "Numeric precision of the region coordinates in the output. Must be strictly positive."
);

DEFINE_double
(
  region_opacity,
  -1,
  "Opacity of the regions in the output. Must be no larger than 1."
  " For negative values, the input opacity is maintained."
  " The regions are otherwise drawn with the same style as the input regions."
);

DEFINE_double
(
  bead_opacity,
  1,
  "Opacity of the necklace beads in the output. Must be in the range [0, 1]."
  " The necklace beads are drawn with roughly the same style as the input regions."
  " However, the boundaries will be hidden for transparant beads."
  // The reason for hiding the boundary is that it has undesirable interaction with the drop shadow filter applied to the beads.
);

DEFINE_bool
(
  draw_necklace_curve,
  true,
  "Whether to draw the necklace shape in the output."
);

DEFINE_bool
(
  draw_necklace_kernel,
  false,
  "Whether to draw the necklace kernel in the output."
);

DEFINE_bool
(
  draw_bead_ids,
  true,
  "Whether to draw the region ID in each bead in the output."
);

DEFINE_bool
(
  draw_feasible_intervals,
  false,
  "Whether to draw the feasible intervals in the output."
);

DEFINE_bool
(
  draw_valid_intervals,
  false,
  "Whether to draw the valid intervals in the output."
);

DEFINE_bool
(
  draw_region_angles,
  false,
  "Whether to draw a line through the region centroids in the output."
);

DEFINE_bool
(
  draw_bead_angles,
  false,
  "Whether to draw a line to the bead centers in the output."
);


void ValidateFlags(geoviz::necklace_map::Parameters& parameters, geoviz::WriterOptions::Ptr& write_options)
{
  bool correct = true;
  LOG(INFO) << "necklace_map_cla flags:";

  using Closure = validate::Closure;
  using BoundSide = validate::BoundSide;

  // Note that we mainly print flags to enable reproducibility.
  // Other flags are validated, but only printed if not valid.
  // Note that we may skip some low-level flags that almost never change.

  // There must be input geometry and input numeric data.
  correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(in_geometry_filename), validate::ExistsFile);
  correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(in_data_filename), validate::ExistsFile);
  correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(in_value_name), validate::NotEmpty);

  // Note that we allow overwriting existing output.
  correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(out_filename), validate::EmptyOr<validate::IsFile>);

  // Interval parameters.
  {
    correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(interval_type), geoviz::IntervalTypeParser(parameters.interval_type));

    validate::MakeRangeCheck(0.0, M_PI)(FLAGS_centroid_interval_length_rad);
    parameters.centroid_interval_length_rad = FLAGS_centroid_interval_length_rad;
    parameters.ignore_point_regions = FLAGS_ignore_point_regions;
  }

  // Scale factor optimization parameters.
  {
    correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(order_type), geoviz::OrderTypeParser(parameters.order_type));

    correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(buffer_rad), validate::MakeRangeCheck(0.0, M_PI));
    parameters.buffer_rad = FLAGS_buffer_rad;
  }

  // Placement parameters.
  {
    correct &= CheckAndPrintFlag
    (
      FLAGS_NAME_AND_VALUE(aversion_ratio),
      validate::MakeRangeCheck<Closure::kOpen, Closure::kClosed>(0.0, 1.0)
    );
    parameters.aversion_ratio = FLAGS_aversion_ratio;
  }

  // Output parameters.
  using Options = geoviz::WriterOptions;
  write_options = Options::Default();
  {
    correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(pixel_width), validate::IsStrictlyPositive<int32_t>());
    write_options->pixel_width = FLAGS_pixel_width;

    correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(region_coordinate_precision), validate::IsStrictlyPositive<int32_t>());
    write_options->region_precision = FLAGS_region_coordinate_precision;

    correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(region_opacity), validate::MakeUpperBoundCheck(1.0));
    write_options->region_opacity = FLAGS_region_opacity;

    correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(bead_opacity), validate::MakeRangeCheck(0.0, 1.0));
    write_options->bead_opacity = FLAGS_bead_opacity;

    PrintFlag(FLAGS_NAME_AND_VALUE(draw_necklace_curve));
    write_options->draw_necklace_curve = FLAGS_draw_necklace_curve;

    PrintFlag(FLAGS_NAME_AND_VALUE(draw_necklace_kernel));
    write_options->draw_necklace_kernel = FLAGS_draw_necklace_kernel;

    PrintFlag(FLAGS_NAME_AND_VALUE(draw_bead_ids));
    write_options->draw_bead_ids = FLAGS_draw_bead_ids;
  }

  // Debug parameters.
  {
    write_options->draw_feasible_intervals = FLAGS_draw_feasible_intervals;
    write_options->draw_valid_intervals = FLAGS_draw_valid_intervals;
    write_options->draw_region_angles = FLAGS_draw_region_angles;
    write_options->draw_bead_angles = FLAGS_draw_bead_angles;
  }

  correct &= CheckAndPrintFlag( FLAGS_NAME_AND_VALUE( log_dir ), validate::EmptyOr<validate::IsDirectory> );
  PrintFlag( FLAGS_NAME_AND_VALUE( stderrthreshold ) );
  PrintFlag( FLAGS_NAME_AND_VALUE( v ) );

  if( !correct ) LOG(FATAL) << "Errors in flags; Terminating.";
}

bool ReadData(std::vector<geoviz::necklace_map::MapElement::Ptr>& elements)
{
  geoviz::DataReader data_reader;
  return data_reader.ReadFile(FLAGS_in_data_filename, FLAGS_in_value_name, elements);
}

bool ReadGeometry
(
  std::vector<geoviz::necklace_map::MapElement::Ptr>& elements,
  std::vector<geoviz::necklace_map::Necklace::Ptr>& necklaces
)
{
  geoviz::SvgReader svg_reader;
  return svg_reader.ReadFile(FLAGS_in_geometry_filename, elements, necklaces);
}

void WriteOutput
(
  const std::vector<geoviz::necklace_map::MapElement::Ptr>& elements,
  const std::vector<geoviz::necklace_map::Necklace::Ptr>& necklaces,
  const geoviz::Number& scale_factor,
  const geoviz::WriterOptions::Ptr& write_options
)
{
  geoviz::NecklaceWriter writer;
  if (FLAGS_out_website)
    writer.Write(elements, necklaces, scale_factor, write_options, std::cout);

  if (!FLAGS_out_filename.empty())
  {
    std::ofstream out(FLAGS_out_filename);
    writer.Write(elements, necklaces, scale_factor, write_options, out);
    out.close();
  }
}


int main(int argc, char **argv)
{
  InitApplication
  (
    argc,
    argv,
    "Command line application that exposes the functionality of the GeoViz necklace map.",
    {"--in_geometry_filename=<file>", "--in_data_filename=<file>", "--in_value_name=<column>"}
  );


  // Validate the settings.
  geoviz::necklace_map::Parameters parameters;
  geoviz::WriterOptions::Ptr write_options;
  ValidateFlags(parameters, write_options);


  geoviz::Timer time;

  using MapElement = geoviz::necklace_map::MapElement;
  using Necklace = geoviz::necklace_map::Necklace;
  std::vector<MapElement::Ptr> elements;
  std::vector<Necklace::Ptr> necklaces;


  // Read the data and geometry.
  const bool success_read_data = ReadData(elements);
  const bool success_read_svg = ReadGeometry(elements, necklaces);
  CHECK(success_read_svg) << "Terminating program.";
  const double time_read = time.Stamp();


  // Compute the optimal scale factor and placement.
  const geoviz::Number scale_factor = success_read_data ? ComputeScaleFactor(parameters, elements, necklaces) : 0;
  LOG(INFO) << "Computed scale factor: " << scale_factor;
  const double time_compute = time.Stamp();


  // Write the output.
  WriteOutput(elements, necklaces, scale_factor, write_options);
  const double time_write = time.Stamp();

  const double time_total = time.Span();

  LOG(INFO) << "Time cost (read files): " << time_read;
  LOG(INFO) << "Time cost (compute NM): " << time_compute;
  LOG(INFO) << "Time cost (serialize):  " << time_write;
  LOG(INFO) << "Time cost (total):      " << time_total;

  return 0;
}
