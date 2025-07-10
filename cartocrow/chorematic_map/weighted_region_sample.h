#ifndef CARTOCROW_SAMPLE_H
#define CARTOCROW_SAMPLE_H

#include "weighted_point.h"

namespace cartocrow::chorematic_map {
template <class K> class WeightedRegionSample {
  public:
	using RegionWeight = std::unordered_map<std::string, double>;
	using AssignWeight = std::function<WeightedPoint(const Point<K>&, const RegionWeight&)>;

	std::vector<Point<K>> m_points;

  private:
	AssignWeight m_assignWeight;

  public:
	WeightedRegionSample() = default;

	template <class InputIterator>
	WeightedRegionSample(InputIterator begin, InputIterator end, AssignWeight assignWeight) :
	      m_assignWeight(std::move(assignWeight)), m_points(begin, end) {};

	explicit WeightedRegionSample(AssignWeight assignWeight) : m_assignWeight(std::move(assignWeight)) {};

	void setAssignWeightFunction(AssignWeight assignWeight) {
		m_assignWeight = std::move(assignWeight);
	}

	template <class OutputIterator>
	void weightedPoints(OutputIterator out, const RegionWeight& regionWeight) const {
		for (const auto& point : m_points) {
			*out++ = m_assignWeight(point, regionWeight);
		}
	}
};
}

#endif //CARTOCROW_SAMPLE_H
