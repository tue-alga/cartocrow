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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 13-02-2020
*/

#include "parsers.h"



IntervalTypeParser::IntervalTypeParser(IntervalType& type) :
  type(type)
{}

bool IntervalTypeParser::operator()(const std::string& str) const
{
  if (str == kCentroid)
  {
    type = IntervalType::kCentroid;
    return true;
  }
  if (str == kWedge)
  {
    type = IntervalType::kWedge;
    return true;
  }
  return false;
}

std::string IntervalTypeParser::Serialize() const
{
  switch (type)
  {
    case IntervalType::kCentroid:
      return kCentroid;
    case IntervalType::kWedge:
      return kWedge;
  }
}


OrderTypeParser::OrderTypeParser(OrderType& type) :
  type(type)
{}

bool OrderTypeParser::operator()(const std::string& str) const
{
  if (str == kFixed)
  {
    type = OrderType::kFixed;
    return true;
  }
  if (str == kAny)
  {
    type = OrderType::kAny;
    return true;
  }
  if (str == kHeuristic)
  {
    type = OrderType::kHeuristicAny;
    return true;
  }
  return false;
}

std::string OrderTypeParser::Serialize() const
{
  switch (type)
  {
    case OrderType::kFixed:
      return kFixed;
    case OrderType::kAny:
      return kAny;
    case OrderType::kHeuristicAny:
      return kHeuristic;
  }
}
