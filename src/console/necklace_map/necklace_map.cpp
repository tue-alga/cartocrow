/*
Application to expose the functionality of the library by the same name.
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
#include <iomanip>
#include <iostream>
#include <memory>
#include <tinyxml2.h>
#include <unordered_map>
#include <vector>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <glog/logging.h>

#include <cmake/geoviz_config.h>
#include <geoviz/necklace_map/necklace_map.h>

#include "console/common/utils_cla.h"
#include "console/common/detail/svg_polygon_parser.h"
#include "console/common/detail/svg_visitor.h"


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












 class SvgNecklaceMapVisitor : public geoviz::detail::SvgVisitor
{
 public:
  SvgNecklaceMapVisitor
  (
    std::vector<Region>& regions,
    std::shared_ptr<NecklaceType>& necklace,
    std::unordered_map<std::string, size_t>& lookup_
  );

 private:


  bool visitCircle
  (
    const Point& center,
    const Number& radius,
    const tinyxml2::XMLAttribute* attributes
  );

  bool visitPath
  (
    const std::string& commands,
    const tinyxml2::XMLAttribute* attributes
  );



  bool SetCircleNecklace(const Point& center, const Number& radius);
  bool SetArcNecklace(const std::string& commands);
  bool SetGenericNecklace(const std::string& commands);

  bool AddRegion(const std::string& commands, const std::string& id, const std::string& style);

  std::vector<Region>& regions_;
  std::shared_ptr<NecklaceType>& necklace_;

  std::unordered_map<std::string, size_t>& lookup_;
};

SvgNecklaceMapVisitor::SvgNecklaceMapVisitor
(
  std::vector<Region>& regions,
  std::shared_ptr<NecklaceType>& necklace,
  std::unordered_map<std::string, size_t>& lookup
)
  : regions_(regions), necklace_(necklace), lookup_(lookup) {}


bool SvgNecklaceMapVisitor::visitCircle
(
  const Point& center,
  const Number& radius,
  const tinyxml2::XMLAttribute* attributes
)
{
  // Circles must be necklaces and necklaces must be dashed.
  std::string style;
  CHECK(findAttribute(attributes, "style", style));
  CHECK(style.find("stroke-dasharray") != std::string::npos);

  SetCircleNecklace(center, radius);
  return false;
}

bool SvgNecklaceMapVisitor::visitPath
(
  const std::string& commands,
  const tinyxml2::XMLAttribute* attributes
)
{
  CHECK(!commands.empty());

  std::string style, id;
  if
  (
    findAttribute(attributes, "style", style) &&
    style.find("stroke-dasharray") != std::string::npos
  )
  {
    // All dashed elements are necklaces.
    // Note that this may have to change into some identifying attribute.
    if (commands.find_first_of("LlZzCcQqSsTt") == std::string::npos)
      return SetArcNecklace(commands);
    else
      return SetGenericNecklace(commands);
  }
  else if (findAttribute(attributes, "id", id))
  {
    // Path elements with an ID are regions.
    CHECK(!id.empty());
    return AddRegion(commands, id, style);;
  }

  // Traverse other elements.
  return true;
}





bool SvgNecklaceMapVisitor::SetCircleNecklace(const Point& center, const Number& radius)
{
  necklace_ = std::make_shared<CircleNecklace>(Circle(center, radius * radius));
  return true;
}

bool SvgNecklaceMapVisitor::SetArcNecklace(const std::string& commands)
{
  LOG(FATAL) << "Not implemented yet.";
  return true;
}

bool SvgNecklaceMapVisitor::SetGenericNecklace(const std::string& commands)
{
  LOG(FATAL) << "Not implemented yet.";
  return true;
}

bool SvgNecklaceMapVisitor::AddRegion(const std::string& commands, const std::string& id, const std::string& style)
{
  // Get the region with the given ID, or create a new one if it does not yet exist.
  const size_t n = lookup_.insert({id, regions_.size()}).first->second;
  if (n == regions_.size()) regions_.emplace_back(id);
  Region& region = regions_[n];
  CHECK_EQ(id, region.id);

  region.style = style;

  // Interpret the commands as a region.
  geoviz::detail::SvgPolygonConverter converter(region.shape);
  geoviz::detail::SvgPathParser()(commands, converter);
  return true;
}






bool readNecklaceMapSvg
(
  const std::string& filename,
  std::vector<Region>& regions,
  std::shared_ptr<NecklaceType>& necklace,
  std::unordered_map<std::string, size_t>& lookup
)
{
  // Note that while the SVG input is stored in an XML file, it does not make use of the main feature that makes XML preferable over JSON: validatibility (i.e. assigning a schema).
  // This means that we do not need to use a comprehensive XML library such as xerces; we can instead use a lightweight library such as tinyxml.

  tinyxml2::XMLDocument doc;
  tinyxml2::XMLError result = doc.LoadFile( filename.c_str() );
  //tinyxml2::XMLError result = doc.Parse( content ); // This would be how to parse a string instead of a file using tinyxml2
  if (result != tinyxml2::XML_SUCCESS) return false;

  SvgNecklaceMapVisitor visitor(regions, necklace, lookup);
  doc.Accept(&visitor);

  return true;
}

std::string toString(const Region::PolygonSet& shape, const Vector& offset = Vector(0, 0))
{
  std::stringstream sout;
  sout << std::setprecision(9);  // TODO(tvl) we should align this precision with the maximum expected precision of the data. Probably should be a parameter with default.

  for (const Polygon& polygon : shape)
  {
    for
    (
      Polygon::Vertex_const_iterator point_iter = polygon.vertices_begin();
      point_iter != polygon.vertices_end();
      ++point_iter
    )
    {
      const Point point = *point_iter + offset;

      if (point_iter == polygon.vertices_begin())
        sout << " M ";
      else
        sout << " L ";

      sout << point.x() << " " << point.y();
    }

    if (1 < polygon.size() && polygon.vertices_begin() != --polygon.vertices_end())
      sout << " Z";
  }

  if (sout.str().empty()) return "";
  return sout.str().substr(1);
}

bool writeDummySvg
(
  const std::vector<Region>& regions,
  const std::shared_ptr<NecklaceType>& necklace,
  std::string& svg
)
{
  // Create dummy necklace glyphs and write everything to an SVG string.

  std::shared_ptr<CircleNecklace> circle_necklace = std::static_pointer_cast<CircleNecklace>(necklace);
  const Point& kernel = circle_necklace->getKernel();
  const Number radius = CGAL::sqrt(circle_necklace->shape.squared_radius());
  std::vector<Point> centers =
  {
    kernel + Vector(radius, 0),
    kernel + Vector(0, radius),
    kernel + Vector(-radius, 0),
    kernel + Vector(0, -radius)
  };
  std::vector<Number> radii = { 5, 5.5, 4.5, 6 };

  Number bounds[] = {kernel.x(), kernel.y(), kernel.x(), kernel.y()};

  // Necklace
  for (size_t n = 0; n < centers.size(); ++n)
  {
    bounds[0] = std::min(bounds[0], centers[n].x() - radii[n]);
    bounds[1] = std::min(bounds[1], centers[n].y() - radii[n]);
    bounds[2] = std::max(bounds[2], centers[n].x() + radii[n]);
    bounds[3] = std::max(bounds[3], centers[n].y() + radii[n]);
  }

  // Regions.
  for (const Region& region : regions)
  {
    for (const Polygon& polygon : region.shape)
    {
      for
      (
        Polygon::Vertex_const_iterator point_iter = polygon.vertices_begin();
        point_iter != polygon.vertices_end();
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
    view_str << bounds[0] << " " << bounds[1] << " " << width << " " << height;
    printer.PushAttribute("viewBox", view_str.str().c_str());
  }

  printer.PushAttribute("version", "1.1");

  {
    std::stringstream bounds_str;
    bounds_str << "[[" << bounds[1] << "," << bounds[0] << "],[" << bounds[3] << "," << bounds[2] << "]]";  // Note, bounds expected in latitude-longitude.
    printer.PushAttribute("bounds", bounds_str.str().c_str());
    //printer.PushAttribute("bounds", "[[52.356,4.945],[52.354,4.947]]");  // TODO(tvl) Note: lat-lon!
  }

  const Vector offset;//(-bounds[0], -bounds[1]);

  for (const Region& region : regions)
  {
    printer.OpenElement("path");
    printer.PushAttribute("style", region.style.c_str());
    printer.PushAttribute("d", toString(region.shape, offset).c_str());
    printer.PushAttribute("id", region.id.c_str());
    printer.CloseElement();
  }

  printer.OpenElement("circle");
  printer.PushAttribute("style", "fill:none;stroke-width:0.4;stroke-linecap:butt;stroke-linejoin:round;stroke:rgb(0%,0%,0%);stroke-opacity:1;stroke-miterlimit:10;");
  printer.PushAttribute("cx", kernel.x() + offset.x());
  printer.PushAttribute("cy", kernel.y() + offset.y());
  printer.PushAttribute("r", radius);
  //printer.PushAttribute("transform", "matrix(13.695085,0,0,-13.695085,-3559.352703,7300.288543)");
  printer.CloseElement();

  for (size_t n = 0; n < centers.size(); ++n)
  {
    printer.OpenElement("circle");
    printer.PushAttribute("style",
                          "fill-rule:evenodd;fill:rgb(80%,80%,80%);fill-opacity:1;stroke-width:0.2;stroke-linecap:butt;stroke-linejoin:round;stroke:rgb(0%,0%,0%);stroke-opacity:1;stroke-miterlimit:10;"
                         );
    printer.PushAttribute("cx", centers[n].x() + offset.x());
    printer.PushAttribute("cy", centers[n].y() + offset.y());
    printer.PushAttribute("r", radii[n]);
    printer.CloseElement();
  }

  printer.PushText("Sorry, your browser does not support the svg tag.");
  printer.CloseElement();
  svg = printer.CStr();
}


int main(int argc, char **argv)
{
  initApplication
  (
    argc,
    argv,
    "Command line application that exposes the functionality of the GeoViz necklace map.",
    {"<some arg>", "<another arg>"}
  );

  // Writing to the standard output is reserved for text that should be returned to a calling website.
  FLAGS_logtostderr = true;

  //std::cerr << "Test" << std::endl;

  std::vector<Region> regions;
  std::shared_ptr<NecklaceType> necklace;
  std::unordered_map<std::string, size_t> region_lookup;
  readNecklaceMapSvg
  (
    "/storage/GeoViz/wwwroot/data/Example_wEU/wEU_svg.xml",
    regions,
    necklace,
    region_lookup
  );

  std::string out;
  writeDummySvg
  (
    regions,
    necklace,
    out
  );

  std::cout << out;

  //LOG(INFO) << "Finishing example run";
  exit(0);





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

  LOG(INFO) << "Args:";
  for (int i = 1; i < argc; ++i)
    LOG(INFO) << "\t" << argv[i];

  LOG(INFO) << "GeoViz version: " << GEOVIZ_VERSION;

  std::cout << "<div>";
  std::cout << "<!--To be loaded as support card.-->";
  std::cout << "<h1>Application-based component</h1>";
  std::cout << "<p>This page is just to test running a server-side application using PHP.</p>";
  std::cout << "<p>The following command line arguments were caught:<br>";
  for (int i = 1; i < argc; ++i)
    std::cout << argv[i] << "<br>";
  std::cout << "</p>";
  std::cout << "</div>";

  return 0;
}
