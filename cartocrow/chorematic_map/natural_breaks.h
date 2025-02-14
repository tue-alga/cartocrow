#ifndef CARTOCROW_NATURAL_BREAKS_H
#define CARTOCROW_NATURAL_BREAKS_H

#include "natural_breaks_external.h"

namespace cartocrow::chorematic_map {
template<class InputIterator, class OutputIterator>
void natural_breaks(InputIterator begin, InputIterator end, OutputIterator out, int nBins) {
    std::vector<double> values(begin, end);
    details::ValueCountPairContainer sortedUniqueValueCounts;
    details::GetValueCountPairs(sortedUniqueValueCounts, &values[0], values.size());

    if (sortedUniqueValueCounts.size() <= nBins) {
        for (int i = 1; i < sortedUniqueValueCounts.size(); ++i) {
            *out++ = sortedUniqueValueCounts[i].first;
        }
        int remaining = nBins - 1 - (sortedUniqueValueCounts.size() - 1);
        for (int i = 0; i < remaining; ++i) {
            *out++ = sortedUniqueValueCounts.back().first;
        }
        return;
    }

    details::LimitsContainer resultingbreaksArray;
    details::ClassifyJenksFisherFromValueCountPairs(resultingbreaksArray, nBins, sortedUniqueValueCounts);

    for (int i = 1; i < resultingbreaksArray.size(); ++i) {
        *out++ = resultingbreaksArray[i];
    }
}
}
#endif //CARTOCROW_NATURAL_BREAKS_H
