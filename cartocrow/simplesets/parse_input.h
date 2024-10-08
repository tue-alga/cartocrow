#ifndef CARTOCROW_PARSE_INPUT_H
#define CARTOCROW_PARSE_INPUT_H

#include "cat_point.h"

namespace cartocrow::simplesets {
std::vector<CatPoint> parseCatPoints(const std::string& s);
}

#endif //CARTOCROW_PARSE_INPUT_H
