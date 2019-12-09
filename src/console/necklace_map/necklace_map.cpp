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
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <glog/logging.h>

#include <cmake/geoviz_config.h>
#include <geoviz/necklace_map/necklace_map.h>

#include "console/common/utils_cla.h"
#include "console/common/detail/svg_polygon_parser.h"
#include "console/common/detail/svg_visitor.h"

#include "console/necklace_map/svg_reader.h"
#include "console/necklace_map/data_reader.h"



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
  const std::vector<NecklaceElement>& elements,
  std::string& svg
)
{
  //std::vector<Number> angles_rad = {0, M_PI_4, M_PI_2, M_PI_2 + M_PI_4, M_PI, M_PI + M_PI_2};




  const std::shared_ptr<NecklaceType>& necklace = elements[0].necklace;

  // Create dummy necklace glyphs and write everything to an SVG string.
  const Point& kernel = necklace->kernel();

  //std::shared_ptr<CircleNecklace> circle_necklace = std::static_pointer_cast<CircleNecklace>(necklace);
  //const Number radius = circle_necklace == nullptr ? 0 : CGAL::sqrt(circle_necklace->shape_.squared_radius());
  const Box bounding_box = necklace->ComputeBoundingBox();
  const Number necklace_radius = (bounding_box.xmax() - bounding_box.xmin()) / 2.0;

  std::vector<Point> centers;
  for (const NecklaceElement& element : elements)
  {
    const std::shared_ptr<NecklaceInterval>& interval = element.glyph.interval;
    const Number angle_rad = (interval->angle_cw_rad() + interval->angle_ccw_rad()) / 2.0;

    centers.emplace_back();
    CHECK(necklace->IntersectRay(angle_rad, centers.back()));
  }


  Number bounds[] = {kernel.x(), kernel.y(), kernel.x(), kernel.y()};

  // Necklace
  for (size_t n = 0; n < centers.size(); ++n)
  {
    const Number radius = std::sqrt(elements[n].value);

    bounds[0] = std::min(bounds[0], centers[n].x() - radius);
    bounds[1] = std::min(bounds[1], centers[n].y() - radius);
    bounds[2] = std::max(bounds[2], centers[n].x() + radius);
    bounds[3] = std::max(bounds[3], centers[n].y() + radius);
  }

  // Regions.
  for (const NecklaceElement& element : elements)
  {
    for (const Polygon_with_holes& polygon : element.region.shape)
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

  for (const NecklaceElement& element : elements)
  {
    const Region& region = element.region;

    printer.OpenElement("path");
    printer.PushAttribute("style", region.style.c_str());
    printer.PushAttribute("d", toString(region.shape).c_str());
    printer.PushAttribute("id", region.id.c_str());
    printer.PushAttribute("transform", transform_str.str().c_str());
    printer.CloseElement();
  }

  printer.OpenElement("circle");
  printer.PushAttribute("style", "fill:none;stroke-width:0.4;stroke-linecap:butt;stroke-linejoin:round;stroke:rgb(0%,0%,0%);stroke-opacity:1;stroke-miterlimit:10;");
  printer.PushAttribute("cx", kernel.x());
  printer.PushAttribute("cy", kernel.y());
  printer.PushAttribute("r", necklace_radius);
  printer.PushAttribute("transform", transform_str.str().c_str());
  printer.CloseElement();

  for (size_t n = 0; n < centers.size(); ++n)
  {
    const std::string style = elements[n].region.style;

    {
      const std::shared_ptr<NecklaceInterval>& interval = elements[n].glyph.interval;

      const size_t from = style.find("fill:");
      const size_t to = style.find(")", from);
      const std::string color = style.substr(from + 5, to - from - 4);
      const std::string interval_style =
        "fill:none;stroke-width:0.4;stroke-linecap:butt;stroke-linejoin:round;stroke:"
        + color +
        ";stroke-opacity:1;stroke-miterlimit:10;";

      const Number my_radius = necklace_radius + (0.4 * (n+1));

      CircleNecklace my_necklace(Circle(kernel, my_radius * my_radius));

      Point from_pt, to_pt;
      my_necklace.IntersectRay(interval->angle_cw_rad(), from_pt);
      my_necklace.IntersectRay(interval->angle_ccw_rad(), to_pt);

      std::stringstream sout;
      sout << std::setprecision(9);  // TODO(tvl) we should align this precision with the maximum expected precision of the data. Probably should be a parameter with default.
      sout << "M " << from_pt.x() << " " << from_pt.y();
      sout << " A " << my_radius << " " << my_radius << " 0 0 1 ";
      sout  << to_pt.x() << " " << to_pt.y();
      std::string path_str = sout.str();

      printer.OpenElement("path");
      printer.PushAttribute("style", interval_style.c_str());
      printer.PushAttribute("d", path_str.c_str());
      printer.PushAttribute("transform", transform_str.str().c_str());
      printer.CloseElement();
    }

    {
      const Number radius = std::sqrt(elements[n].value);

      const size_t from = style.find("fill-opacity:");
      const size_t to = style.find(";", from);
      const std::string glyph_style =
        style.substr(0, from+13)
        + "0.5" +
        style.substr(to);

      printer.OpenElement("circle");
      printer.PushAttribute("style", glyph_style.c_str());
      printer.PushAttribute("cx", centers[n].x());
      printer.PushAttribute("cy", centers[n].y());
      printer.PushAttribute("r", radius);
      printer.PushAttribute("transform", transform_str.str().c_str());
      printer.CloseElement();
    }

    {
      std::stringstream sout;
      sout << std::setprecision(9
                               );  // TODO(tvl) we should align this precision with the maximum expected precision of the data. Probably should be a parameter with default.
      sout << "M " << kernel.x() << " " << kernel.y() << " L " << centers[n].x() << " " << centers[n].y();
      std::string path_str = sout.str();

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
  }

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


  const std::string svg_filename = "/storage/GeoViz/wwwroot/data/Example_wEU/wEU_svg.xml";
  const std::string data_filename = "/storage/GeoViz/wwwroot/data/Example_wEU/wEU.txt";
  const std::string value_name = "value";

  std::vector<NecklaceElement> elements;
  std::vector<SvgReader::NecklaceTypePtr> necklaces;

  SvgReader svg_reader
  (
    elements,
    necklaces
  );

  int c = 0;
  do
  {
    LOG_IF(INFO, !svg_reader.Read(svg_filename)) << "Failed to read necklace map geometry file " << svg_filename;
    LOG_IF(INFO, !elements.empty())
      << "Read necklace map geometry file " << svg_filename
      << " (" << elements.size() << " regions; "<< necklaces.size() << " necklaces)";
    // Note(tvl) we should allow the SVG to no contain the necklace: then create the necklace as smallest enclosing circle.
  } while (elements.empty() || 3 < ++c);

  DataReader data_reader(elements);
  LOG_IF(INFO, !data_reader.Read(data_filename, value_name)) << "Failed to read necklace map data file " << data_filename;

  std::unique_ptr<IntervalGenerator> make_interval(new IntervalCentroidGenerator(0.1 * M_PI));
  (*make_interval)(elements);;






  std::string out;
  writeDummySvg
  (
    elements,
    out
  );

  std::cout << out;

  std::ofstream fout("/storage/GeoViz/wwwroot/data/Example_wEU/test_out.xml");
  fout << out;
  fout.close();

  return 0;
}
