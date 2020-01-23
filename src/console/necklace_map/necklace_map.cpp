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

// Note dependencies:
// - glog
// - gflags

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include <cmake/geoviz_config.h>
#include <geoviz/necklace_map/necklace_map.h>

#include "console/common/utils_cla.h"
#include "console/common/detail/svg_polygon_parser.h"
#include "console/common/detail/svg_visitor.h"

#include "console/necklace_map/svg_reader.h"
#include "console/necklace_map/data_reader.h"


DEFINE_double(buffer_rad, 0, "Minimum distance between the necklace beads (in radians). Must be in range [0, pi]. Note that large values are likely to force the necklace bead area to 0. Also note that values close to 0 are a lot more influential. A 4th degree scaling is recommended.");

//TODO(tvl) rename glyph_aversion?
DEFINE_double(glyph_repulsion, 0.001, "Measure for repulsion between the necklace beads. Must be in the range (0, 1]. Note that values close to 0 are a lot more influential, so a 4th degree scaling is recommended.");

DEFINE_bool(draw_intervals, false, "Whether to draw the feasible and valid intervals.");

DEFINE_bool(debug_out, false, "Whether to generate additional debug output.");

DEFINE_double(centroid_interval_length_rad, 0.2 * M_PI, "The length of the centroid intervals (in radians).");


// Source files for
// * generic SVG parser
// * necklace specific parser and reader.



// When reading example data:
// .mrk files: number of IDs; per line: ID.
// .txt files: number of countries; per country: ID, name, necklace value, 'flow value'
// .ipe files: country and necklace shapes (check java implementation for format...)
// as input, I expect either an XML file with SVG content, or (later on) that content as command line argument...
// another input is the data values per region and possibly other aspects like the color.

//TODO(tvl) extract string literals and make them static const

using namespace geoviz;  //TODO(tvl) remove: name types explicitly!
using namespace geoviz::necklace_map;  //TODO(tvl) remove: name types explicitly!






std::string toString(const Region::PolygonSet& shape)
{
  std::stringstream sout;
  sout << std::setprecision(9);  // TODO(tvl) we should align this precision with the maximum expected precision of the data. Probably should be a parameter with default.

  for (const Polygon_with_holes& polygon : shape)
  {
    for
    (
      Polygon::Vertex_const_iterator point_iter = polygon.outer_boundary().vertices_begin();
      point_iter != polygon.outer_boundary().vertices_end();
      ++point_iter
    )
    {
      if (point_iter == polygon.outer_boundary().vertices_begin())
        sout << " M ";
      else
        sout << " L ";

      sout << point_iter->x() << " " << point_iter->y();
    }

    if
    (
      1 < polygon.outer_boundary().size() &&
      polygon.outer_boundary().vertices_begin() != --polygon.outer_boundary().vertices_end()
    )
      sout << " Z";
  }

  if (sout.str().empty()) return "";
  return sout.str().substr(1);
}

bool writeDummySvg
(
  const std::vector<MapElement::Ptr>& elements,  //TODO(tvl) rename "elements"
  const Necklace::Ptr& necklace,
  const Number& scale_factor,
  std::string& svg,
  bool write_debug_info = false
)
{
  const double symbol_opacity = write_debug_info ? 0.5 : 1;


  //std::vector<Number> angles_rad = {0, M_PI_4, M_PI_2, M_PI_2 + M_PI_4, M_PI, M_PI + M_PI_2};
  //const std::vector<MapElement::Ptr>& elements = necklace->beads;
  const std::vector<Bead::Ptr>& beads = necklace->beads;


  // Create dummy necklace beads and write everything to an SVG string.
  const Point& kernel = necklace->shape->kernel();

  //std::shared_ptr<CircleNecklace> circle_necklace = std::static_pointer_cast<CircleNecklace>(necklace);
  //const Number radius = circle_necklace == nullptr ? 0 : CGAL::sqrt(circle_necklace->shape_.squared_radius());
  const Box bounding_box = necklace->shape->ComputeBoundingBox();
  const Number necklace_radius = (bounding_box.xmax() - bounding_box.xmin()) / 2.0;



  Number bounds[] = {bounding_box.xmin(), bounding_box.ymin(), bounding_box.xmax(), bounding_box.ymax()};

  // Necklace
  for (const Bead::Ptr& bead : beads)
  {
    CHECK_NOTNULL(bead);
    Point symbol_center;
    CHECK(necklace->shape->IntersectRay(bead->angle_rad, symbol_center));

    const Number& radius = scale_factor * bead->radius_base;

    bounds[0] = std::min(bounds[0], symbol_center.x() - radius);
    bounds[1] = std::min(bounds[1], symbol_center.y() - radius);
    bounds[2] = std::max(bounds[2], symbol_center.x() + radius);
    bounds[3] = std::max(bounds[3], symbol_center.y() + radius);
  }

  // Regions.
  for (const MapElement::Ptr& element : elements)
  {
    for (const Polygon_with_holes& polygon : element->region.shape)
    {
      for
      (
        Polygon::Vertex_const_iterator point_iter = polygon.outer_boundary().vertices_begin();
        point_iter != polygon.outer_boundary().vertices_end();
        ++point_iter
      )
      {
        bounds[0] = std::min(bounds[0], point_iter->x());
        bounds[1] = std::min(bounds[1], point_iter->y());
        bounds[2] = std::max(bounds[2], point_iter->x());
        bounds[3] = std::max(bounds[3], point_iter->y());
      }
    }
  }

  // Buffer.
  const Number buffer = 2;
  bounds[0] -= buffer;
  bounds[1] -= buffer;
  bounds[2] += buffer;
  bounds[3] += buffer;

  const Number width = bounds[2] - bounds[0];
  const Number height = bounds[3] - bounds[1];
  const Number pixel_width = 500;
  const Number pixel_height = pixel_width * height / width;
  const Number unit_px = width / pixel_width;

  const Number transform_scale = 1;
  std::stringstream transform_str;
  transform_str << "matrix(" <<
    transform_scale << ",0,0," <<
   -transform_scale << "," <<
   -transform_scale * bounds[0] << "," <<
    transform_scale * bounds[3] << ")";


  tinyxml2::XMLPrinter printer;
  printer.PushComment
  (
    "Copyright 2019 Netherlands eScience Center and TU Eindhoven\n"
    "Licensed under the Apache License, version 2.0. See LICENSE for details."
  );

  printer.OpenElement("svg");
  printer.PushAttribute("xmlns", "http://www.w3.org/2000/svg");
  printer.PushAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
  printer.PushAttribute("width", pixel_width);
  printer.PushAttribute("height", pixel_height);

  {
    std::stringstream view_str;
    //view_str << bounds[0] << " " << bounds[1] << " " << width << " " << height;
    view_str << "0 0 " << width << " " << height;
    printer.PushAttribute("viewBox", view_str.str().c_str());
  }

  printer.PushAttribute("version", "1.1");

  {
    std::stringstream bounds_str;
    bounds_str << "[[" << bounds[1] << "," << bounds[0] << "],[" << bounds[3] << "," << bounds[2] << "]]";  // Note, bounds expected in latitude-longitude.
    printer.PushAttribute("bounds", bounds_str.str().c_str());
    //printer.PushAttribute("bounds", "[[52.356,4.945],[52.354,4.947]]");  // TODO(tvl) Note: lat-lon!
  }

  {
    // Define drop shadow filter.

    printer.OpenElement("defs");
    printer.OpenElement("filter");
    printer.PushAttribute("id", "filterDropShadow");
    printer.PushAttribute("filterUnits", "userSpaceOnUse");
    {
      double mult = 0.9 / symbol_opacity;
      std::stringstream stream;
      stream << "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 " << mult << " 0";
      const std::string opaque_matrix = stream.str();

      printer.OpenElement("feColorMatrix");
      printer.PushAttribute("in", "SourceAlpha");
      printer.PushAttribute("type", "matrix");
      printer.PushAttribute("values", opaque_matrix.c_str());
      printer.PushAttribute("result", "sourceOblique");
      printer.CloseElement(); // feColorMatrix
    }
    {
      const Number extent_px = 2;
      const Number extent = extent_px * unit_px;

      std::stringstream stream;
      stream << extent;
      const std::string blur = stream.str();
      const std::string dx = blur;
      const std::string dy = blur;

      printer.OpenElement("feGaussianBlur");
      printer.PushAttribute("in", "sourceOblique");
      printer.PushAttribute("stdDeviation", blur.c_str());
      printer.PushAttribute("result", "blur");
      printer.CloseElement(); // feGaussianBlur
      printer.OpenElement("feOffset");
      printer.PushAttribute("in", "blur");
      printer.PushAttribute("dx", dx.c_str());
      printer.PushAttribute("dy", dy.c_str());
      printer.PushAttribute("result", "offsetBlur");
      printer.CloseElement(); // feOffset
    }
    printer.OpenElement("feComposite");
    printer.PushAttribute("in", "offsetBlur");
    printer.PushAttribute("in2", "sourceOblique");
    printer.PushAttribute("operator", "xor");
    if (write_debug_info)
    {
      // Subtract the original from the shadow.
      printer.PushAttribute("result", "offsetBlurAndSource");
      printer.CloseElement(); // feComposite
      printer.OpenElement("feComposite");
      printer.PushAttribute("in", "sourceOblique");
      printer.PushAttribute("in2", "offsetBlurAndSource");
      printer.PushAttribute("operator", "arithmetic");
      printer.PushAttribute("k1", "0");
      printer.PushAttribute("k2", "-1");
      printer.PushAttribute("k3", "1");
      printer.PushAttribute("k4", "0");
      printer.PushAttribute("result", "dropShadow");
    }
    else
    {
      printer.PushAttribute("result", "dropShadow");
    }
    printer.CloseElement(); // feComposite
    printer.OpenElement("feMerge");
    printer.OpenElement("feMergeNode");
    //printer.PushAttribute("in", "dropShadow");
    printer.PushAttribute("in", "dropShadow");
    printer.CloseElement(); // feMergeNode
    printer.OpenElement("feMergeNode");
    printer.PushAttribute("in", "SourceGraphic");
    printer.CloseElement(); // feMergeNode
    printer.CloseElement(); // feMerge
    printer.CloseElement(); // filter
    printer.CloseElement(); // defs
  }

  // Write regions.
  for (const MapElement::Ptr& element : elements)
  {
    const Region& region = element->region;

    printer.OpenElement("path");
    printer.PushAttribute("style", region.style.c_str());
    printer.PushAttribute("d", toString(region.shape).c_str());
    printer.PushAttribute("id", region.id.c_str());
    printer.PushAttribute("transform", transform_str.str().c_str());
    printer.CloseElement();
  }

  // Write necklace.
  printer.OpenElement("circle");
  printer.PushAttribute("style", "fill:none;stroke-width:0.4;stroke-linecap:butt;stroke-linejoin:round;stroke:rgb(0%,0%,0%);stroke-opacity:1;stroke-miterlimit:10;");
  printer.PushAttribute("cx", kernel.x());
  printer.PushAttribute("cy", kernel.y());
  printer.PushAttribute("r", necklace_radius);
  printer.PushAttribute("transform", transform_str.str().c_str());
  printer.CloseElement();

  // TODO(tvl) make separate loops to draw the different aspects of the elements (e.g. first draw all intervals, then all lines, etc.).
  size_t c = 0;
  for (const MapElement::Ptr& element : elements)
  {
    const std::string style = element->region.style;
    for (const MapElement::BeadMap::value_type& map_value : element->beads)
    {
      const Bead::Ptr& bead = map_value.second;

      // Interval
      if (write_debug_info)
      {
        const std::shared_ptr<CircleRange>& interval = bead->feasible;

        const size_t from = style.find("fill:");
        const size_t to = style.find(")", from);
        const std::string color = style.substr(from + 5, to - from - 4);
        const std::string interval_style =
          "fill:none;stroke-width:0.4;stroke-linecap:butt;stroke-linejoin:round;stroke:"
          + color +
          ";stroke-opacity:1;stroke-miterlimit:10;";

        const Number my_radius = necklace_radius + (0.4 * ++c);

        CircleNecklace my_necklace(Circle(kernel, my_radius * my_radius));

        std::string path_str;

        if (bead->valid)
        {
          // Write line to interval through bead.

          Point endpoint;
          //CHECK(necklace->IntersectRay(angle_rad, endpoint));
          CHECK(my_necklace.IntersectRay(bead->angle_rad, endpoint));

          {
            std::stringstream sout;
            sout << std::setprecision(
              9
                                     );  // TODO(tvl) we should align this precision with the maximum expected precision of the data. Probably should be a parameter with default.
            sout << "M " << kernel.x() << " " << kernel.y() << " L " << endpoint.x() << " " << endpoint.y();
            path_str = sout.str();
          }

          printer.OpenElement("path");
          printer.PushAttribute
            (
              "style",
              "fill:none;stroke-width:0.2;stroke-linecap:butt;stroke-linejoin:round;stroke:rgb(0%,0%,0%);stroke-opacity:1;stroke-miterlimit:10;"
            );
          printer.PushAttribute("d", path_str.c_str());
          printer.PushAttribute("transform", transform_str.str().c_str());
          printer.CloseElement();
        }

        if (bead->valid)
        {
          // Write allowed wedge.

          //const Number angle_rad = (interval->angle_cw_rad() + interval->angle_ccw_rad()) / 2.0;
          const Number angle_cw_rad = bead->valid->angle_cw_rad();
          const Number angle_ccw_rad = bead->valid->angle_ccw_rad();

          Point endpoint_cw, endpoint_ccw;
          //CHECK(necklace->IntersectRay(angle_rad, endpoint));
          CHECK(my_necklace.IntersectRay(angle_cw_rad, endpoint_cw));
          CHECK(my_necklace.IntersectRay(angle_ccw_rad, endpoint_ccw));

          {
            std::stringstream sout;
            sout << std::setprecision(
              9
                                     );  // TODO(tvl) we should align this precision with the maximum expected precision of the data. Probably should be a parameter with default.
            sout <<
                 "M " << endpoint_cw.x() << " " << endpoint_cw.y() <<
                 " L " << kernel.x() << " " << kernel.y() <<
                 " L " << endpoint_ccw.x() << " " << endpoint_ccw.y();
            path_str = sout.str();
          }

          printer.OpenElement("path");
          printer.PushAttribute
            (
              "style",
              "fill:none;stroke-width:0.2;stroke-linecap:butt;stroke-linejoin:round;stroke:rgb(70%,70%,70%);stroke-opacity:1;stroke-miterlimit:10;"
            );
          printer.PushAttribute("d", path_str.c_str());
          printer.PushAttribute("transform", transform_str.str().c_str());
          printer.CloseElement();
        }


        if(true)
        {
          // Write interval.
          Point from_pt, to_pt;
          my_necklace.IntersectRay(interval->angle_cw_rad(), from_pt);
          my_necklace.IntersectRay(interval->angle_ccw_rad(), to_pt);

          {
            std::stringstream sout;
            sout
              << std::setprecision(9);  // TODO(tvl) we should align this precision with the maximum expected precision of the data. Probably should be a parameter with default.
            sout << "M " << from_pt.x() << " " << from_pt.y();
            sout << " A " << my_radius << " " << my_radius << " 0 0 1 ";
            sout << to_pt.x() << " " << to_pt.y();
            path_str = sout.str();
          }

          printer.OpenElement("path");
          printer.PushAttribute("style", interval_style.c_str());
          printer.PushAttribute("d", path_str.c_str());
          printer.PushAttribute("transform", transform_str.str().c_str());
          printer.CloseElement();
        }
      }
    }
  }

  // Write beads.
  printer.OpenElement("g");
  printer.PushAttribute("filter", "url(#filterDropShadow)");
  for (const MapElement::Ptr& element : elements)
  {
    const std::string style = element->region.style;
    for (const MapElement::BeadMap::value_type& map_value : element->beads)
    {
      const Bead::Ptr& bead = map_value.second;
      if (!bead->valid)
        continue;

      Point symbol_center;
      CHECK(necklace->shape->IntersectRay(bead->angle_rad, symbol_center));
      const Number& radius = scale_factor * bead->radius_base;

      const size_t from = style.find("fill-opacity:");
      const size_t to = style.find(";", from);

      std::stringstream stream;
      stream << style.substr(0, from + 13) << symbol_opacity << style.substr(to);
      const std::string symbol_style = stream.str();

      printer.OpenElement("circle");
      printer.PushAttribute("style", symbol_style.c_str());
      printer.PushAttribute("cx", symbol_center.x());
      printer.PushAttribute("cy", symbol_center.y());
      printer.PushAttribute("r", radius);
      printer.PushAttribute("transform", transform_str.str().c_str());
      printer.CloseElement(); // circle
    }
  }
  printer.CloseElement(); // g

  // Write bead IDs.
  printer.OpenElement("g");
  printer.PushAttribute("font-family", "Verdana");
  {
    const Number font_px = 16;
    const Number font = font_px * unit_px;
    std::stringstream stream;
    stream << font;
    const std::string size = stream.str();
    printer.PushAttribute("font-size", size.c_str());
  }
  for (const MapElement::Ptr& element : elements)
  {
    const std::string id = element->region.id;
    for (const MapElement::BeadMap::value_type& map_value : element->beads)
    {
      const Bead::Ptr& bead = map_value.second;
      if (!bead->valid)
        continue;

      Point symbol_center;
      CHECK(necklace->shape->IntersectRay(bead->angle_rad, symbol_center));

      // Note that the transform argument does not apply to text coordinates.
      const Point transformed_center
      (
        transform_scale * (symbol_center.x() - bounds[0]),
        transform_scale * (bounds[3] - symbol_center.y())
      );

      printer.OpenElement("text");
      printer.PushAttribute("text-anchor", "middle");
      printer.PushAttribute("alignment-baseline", "central");
      {
        std::stringstream stream;
        stream << transformed_center.x();
        const std::string x = stream.str();
        printer.PushAttribute("x", x.c_str());
      }
      {
        std::stringstream stream;
        stream << transformed_center.y();
        const std::string y = stream.str();
        printer.PushAttribute("y", y.c_str());
      }
      //printer.PushAttribute("transform", transform_str.str().c_str());
      printer.PushText(id.c_str());
      printer.CloseElement(); // text
    }
  }
  printer.CloseElement(); // g

  printer.PushText("Sorry, your browser does not support the svg tag.");
  printer.CloseElement();
  svg = printer.CStr();
}

int main(int argc, char **argv)
{
  InitApplication
  (
    argc,
    argv,
    "Command line application that exposes the functionality of the GeoViz necklace map.",
    {"<some arg>", "<another arg>"}
  );

  // Writing to the standard output is reserved for text that should be returned to a calling website.
  FLAGS_logtostderr = true;

  // Note that here is a good place to check the semantic validity of the flags.
  // While this can be done by adding flag validators using gflags,
  // These generally only allow (statically) validating independent flags,
  // they do not allow validating programmatically set flags,
  // and they may throw inconvenient compiler warnings about unused static variables
  // (depending on your implementation).
  // Instead of registering validators, I prefer to add a "ValidateFlags()"
  // method call here that performs all the semantic validation and error messaging
  // (syntactic validation will be done by gflags automatically).
  // For added value, this method could always log the values of important flags
  // to make it easier to reproduce a particular program execution.

  //LOG(INFO) << "GeoViz version: " << GEOVIZ_VERSION;

  // TODO(tvl) when moving to text input as flag, be careful of special characters such as ':', '"', ''', '='!

  const std::string svg_filename = "/storage/GeoViz/wwwroot/data/Example_wEU/wEU_svg.xml";
  const std::string data_filename = "/storage/GeoViz/wwwroot/data/Example_wEU/wEU.txt";
  const std::string value_name = "value";

  std::vector<MapElement::Ptr> elements;
  std::vector<Necklace::Ptr> necklaces;




  // TODO(tvl) capture in method that also does the retries and make stable!
  const int max_tries = 5;
  int c = 0;
  bool success_read_data = false;
  do
  {
    try
    {
      DataReader data_reader(elements);
      success_read_data = data_reader.Read(data_filename, value_name);
    }
    catch (const std::exception& e)
    {
      success_read_data = false;
      LOG(ERROR) << e.what();
    }

    if (success_read_data)
      break;
    else
      LOG(INFO) << "Failed to read necklace map data file " << data_filename;
  } while (max_tries < ++c);

  c = 0;
  bool success_read_svg = false;
  do
  {
    try
    {
      SvgReader svg_reader
      (
        elements,
        necklaces
      );
      success_read_svg = svg_reader.Read(svg_filename);
      LOG_IF(INFO, !elements.empty())
      << "Read necklace map geometry file " << svg_filename
      << " (" << elements.size() << " regions; "<< necklaces.size() << " necklaces)";
      // Note(tvl) we should allow the SVG to not contain the necklace: then create the necklace as smallest enclosing circle.
    }
    catch (const std::exception& e)
    {
      success_read_svg = false;
      LOG(ERROR) << e.what();
    }

    if (success_read_svg)
      break;
    else
      LOG(INFO) << "Failed to read necklace map geometry file " << svg_filename;
  } while (max_tries < ++c);

  // Generate intervals based on the regions and necklaces.
  std::unique_ptr<ComputeFeasibleInterval> compute_feasible_intervals;
  compute_feasible_intervals.reset(new ComputeFeasibleCentroidInterval(FLAGS_centroid_interval_length_rad));
  (*compute_feasible_intervals)(elements);

  // Compute the scaling factor.
  std::unique_ptr<ComputeScaleFactor> compute_scale_factor(new ComputeScaleFactorFixedOrder(FLAGS_buffer_rad));
  const Number scale_factor = (*compute_scale_factor)(necklaces);

  std::unique_ptr<ComputeValidPlacement> compute_valid_placement(new ComputeValidPlacementFixedOrder(scale_factor, FLAGS_glyph_repulsion, FLAGS_buffer_rad));
  (*compute_valid_placement)(necklaces);


  std::string out;
  writeDummySvg
  (
    elements,
    necklaces[0],
    scale_factor,
    out,
    FLAGS_draw_intervals
  );

  std::cout << out;

  if (FLAGS_debug_out)
  {
    std::ofstream fout("/storage/GeoViz/wwwroot/data/Example_wEU/test_out.xml");
    fout << out;
    fout.close();
  }
  return 0;
}
