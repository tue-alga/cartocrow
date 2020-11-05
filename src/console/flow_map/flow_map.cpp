/*
The Flow Map console application implements the algorithmic
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 31-08-2020
*/

#include <fstream>
#include <sstream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <console/common/utils_cla.h>
#include <console/common/utils_flags.h>
#include <geoviz/common/timer.h>
#include <geoviz/flow_map/flow_map.h>


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
  in_structure_filename,
  "",
  "The input tree structure filename."
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
  "Whether to write the output to the standard output stream for the website."
);

DEFINE_double
(
  restricting_angle_rad,
  0.61,
  "Maximum angle between the line connecting the root and any point on a tree arc and arc's tangent line at that point. Must be in the range (0, pi/2)."
);

DEFINE_int32
(
  pixel_width,
  500,
  "Output pixel width. Must be strictly positive."
);

DEFINE_int32
(
  coordinate_precision,
  5,
  "Numeric precision of the coordinates in the output. Must be strictly positive."
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
  flow_opacity,
  1,
  "Opacity of the flow tree in the output. Must be in the range [0, 1]."
);

DEFINE_double
(
  node_opacity,
  1,
  "Opacity of the nodes in the output. Must be in the range [0, 1]."
);


void ValidateFlags(geoviz::flow_map::Parameters& parameters, geoviz::flow_map::WriteOptions::Ptr& write_options)
{
  bool correct = true;
  LOG(INFO) << "flow_map_cla flags:";

  // Note that we mainly print flags to enable reproducibility.
  // Other flags are validated, but only printed if not valid.
  // Note that we may skip some low-level flags that almost never change.
  using namespace validate;

  // There must be input geometry and input numeric data.
  correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(in_geometry_filename), ExistsFile);
  correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(in_data_filename), ExistsFile);
  correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(in_structure_filename), Or<Empty,ExistsFile>);
  correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(in_value_name), Not<Empty>);

  // Note that we allow overwriting existing output.
  correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(out_filename), Or<Empty, MakeAvailableFile>);

  // Flow map parameters.
  {
    correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(restricting_angle_rad), MakeRangeCheck<Closure::kOpen>(0.0, M_PI_2));
    parameters.restricting_angle_rad = FLAGS_restricting_angle_rad;
  }

  // Output parameters.
  using Options = geoviz::flow_map::WriteOptions;
  write_options = Options::Default();
  {
    correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(pixel_width), IsStrictlyPositive<int32_t>());
    write_options->pixel_width = FLAGS_pixel_width;

    correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(coordinate_precision), IsStrictlyPositive<int32_t>());
    write_options->numeric_precision = FLAGS_coordinate_precision;

    correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(region_opacity), MakeUpperBoundCheck(1.0));
    write_options->region_opacity = FLAGS_region_opacity;

    correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(flow_opacity), MakeUpperBoundCheck(1.0));
    write_options->flow_opacity = FLAGS_flow_opacity;

    correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(node_opacity), MakeUpperBoundCheck(1.0));
    write_options->node_opacity = FLAGS_node_opacity;
  }

  correct &= CheckAndPrintFlag( FLAGS_NAME_AND_VALUE( log_dir ), Or<Empty, IsDirectory> );
  PrintFlag( FLAGS_NAME_AND_VALUE( stderrthreshold ) );
  PrintFlag( FLAGS_NAME_AND_VALUE( v ) );

  if( !correct ) LOG(FATAL) << "Errors in flags; Terminating.";
}

bool ReadGeometry(std::vector<geoviz::Region>& context, std::vector<geoviz::flow_map::Place::Ptr>& places)
{
  geoviz::flow_map::SvgReader svg_reader;
  return svg_reader.ReadFile(FLAGS_in_geometry_filename, context, places);
}

bool ReadData(std::vector<geoviz::flow_map::Place::Ptr>& places, size_t& root_index)
{
  geoviz::flow_map::DataReader data_reader;
  return data_reader.ReadFile(FLAGS_in_data_filename, FLAGS_in_value_name, places, root_index);
}

void WriteOutput
(
  const std::vector<geoviz::Region>& context,
  const geoviz::flow_map::FlowTree::Ptr& tree,
  const geoviz::flow_map::WriteOptions::Ptr& write_options
)
{
  geoviz::flow_map::SvgWriter writer;
  if (FLAGS_out_website)
    writer.Write(context, tree, write_options, std::cout);

  if (!FLAGS_out_filename.empty())
  {
    std::ofstream out(FLAGS_out_filename);
    writer.Write(context, tree, write_options, out);
    out.close();
  }
}


int main(int argc, char **argv)
{
  InitApplication
  (
    argc,
    argv,
    "Command line application that exposes the functionality of the GeoViz flow map.",
    {"--in_geometry_filename=<file>", "--in_data_filename=<file>", "--in_value_name=<column>"}
  );


  // Validate the settings.
  geoviz::flow_map::Parameters parameters;
  geoviz::flow_map::WriteOptions::Ptr write_options;
  ValidateFlags(parameters, write_options);


  geoviz::Timer time;

  using Region = geoviz::Region;
  using Place = geoviz::flow_map::Place;
  std::vector<Region> context;
  std::vector<Place::Ptr> places;
  size_t index_root;

  // Read the geometry and data.
  // Note that the regions should be written in the same order as in the input,
  // because some smaller regions may be used to simulate enclaves inside larger regions.
  // This forces the geometry to be read first.
  const bool success_read_svg = ReadGeometry(context, places);
  const bool success_read_data = ReadData(places, index_root);
  CHECK(success_read_svg && success_read_data) << "Terminating program.";
  const double time_read = time.Stamp();

  // Compute flow map.
  geoviz::flow_map::FlowTree::Ptr tree;
  ComputeFlowMap(parameters, places, index_root, tree);
  LOG(INFO) << "Computed flow map";
  const double time_compute = time.Stamp();

  // Write the output.
  WriteOutput(context, tree, write_options);
  const double time_write = time.Stamp();

  const double time_total = time.Span();

  LOG(INFO) << "Time cost (read files): " << time_read;
  LOG(INFO) << "Time cost (compute FM): " << time_compute;
//  LOG(INFO) << "Time cost (serialize):  " << time_write;
  LOG(INFO) << "Time cost (total):      " << time_total;

  return 0;
}
