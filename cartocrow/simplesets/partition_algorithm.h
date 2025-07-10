#ifndef CARTOCROW_PARTITION_ALGORITHM_H
#define CARTOCROW_PARTITION_ALGORITHM_H

#include "types.h"
#include "cat_point.h"
#include "partition.h"
#include "patterns/bank.h"
#include "patterns/island.h"
#include "patterns/matching.h"
#include "patterns/single_point.h"
#include "settings.h"

namespace cartocrow::simplesets {
struct PossibleMergeEvent {
	Number<Inexact> time;
	std::shared_ptr<PolyPattern> p1;
	std::shared_ptr<PolyPattern> p2;
	std::shared_ptr<PolyPattern> result;
	/// We compute expensive delays lazily. This boolean indicates whether all delays have been computed.
	bool final;
};

Number<Inexact> intersectionDelay(const std::vector<CatPoint>& points, const PolyPattern& p1, const PolyPattern& p2,
								  const PolyPattern& result, const GeneralSettings& gs, const PartitionSettings& ps);

/// This function partition the categorical points using a discrete event simulation.
/// Returns a vector of pairs (t_i, P_i) where t_i is the time in the event simulation that partition P_i was created.
std::vector<std::pair<Number<Inexact>, Partition>>
partition(const std::vector<CatPoint>& points, const GeneralSettings& gs, const PartitionSettings& ps, Number<Inexact> maxTime);
}
#endif //CARTOCROW_PARTITION_ALGORITHM_H
