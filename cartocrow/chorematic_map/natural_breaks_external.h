#ifndef CARTOCROW_NATURAL_BREAKS_EXTERNAL_H
#define CARTOCROW_NATURAL_BREAKS_EXTERNAL_H

#include <vector>

namespace cartocrow::chorematic_map::details {
typedef std::size_t SizeT;
typedef SizeT CountType;
typedef std::pair<double, CountType> ValueCountPair;
typedef std::vector<double> LimitsContainer;
typedef std::vector <ValueCountPair> ValueCountPairContainer;

void
ClassifyJenksFisherFromValueCountPairs(LimitsContainer &breaksArray, SizeT k, const ValueCountPairContainer &vcpc);
void GetValueCountPairs(ValueCountPairContainer &vcpc, const double *values, SizeT n);
}
#endif //CARTOCROW_NATURAL_BREAKS_EXTERNAL_H
