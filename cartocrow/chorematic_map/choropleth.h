#ifndef CARTOCROW_CHOROPLETH_H
#define CARTOCROW_CHOROPLETH_H

#include <utility>
#include "../renderer/geometry_painting.h"

#include "../core/region_arrangement.h"

namespace cartocrow::chorematic_map {
class Choropleth {
  public:
	std::shared_ptr<RegionArrangement> m_arr;
	std::shared_ptr<std::unordered_map<std::string, double>> m_data;

	void rebin() {
		// Clear
		m_bins.clear();
		m_regionToBin.clear();

		// Initialize bins
		for (int i = 0; i < m_thresholds.size() + 1; ++i) {
			m_bins.push_back(std::vector<std::string>({}));
		}

		// Fill bins and regionToBin map
		for (auto fit = m_arr->faces_begin(); fit != m_arr->faces_end(); ++fit) {
			auto region = fit->data();
			if (!m_data->contains(region)) continue;
			double value = m_data->at(region);
			int bin = std::distance(m_thresholds.begin(), std::upper_bound(m_thresholds.begin(), m_thresholds.end(), value));
			m_bins[bin].push_back(region);
			m_regionToBin[region] = bin;
		}
	}

	template <class InputIterator>
	Choropleth(std::shared_ptr<RegionArrangement> arr,
	           std::shared_ptr<std::unordered_map<std::string, double>> data,
	           InputIterator thresholdsBegin, InputIterator thresholdsEnd) :
	      m_arr(std::move(arr)), m_data(std::move(data)), m_thresholds(thresholdsBegin, thresholdsEnd) {
		rebin();
	}

	std::optional<int> regionToBin(const std::string& region) const {
		if (!m_regionToBin.contains(region)) return std::nullopt;
		return m_regionToBin.at(region);
	}

	template <class InputIterator>
	void setThresholds(InputIterator begin, InputIterator end) {
		m_thresholds.clear();
		m_thresholds.resize(0);
		std::copy(begin, end, std::back_inserter(m_thresholds));
	}

  private:
	std::vector<double> m_thresholds;
	std::vector<std::vector<std::string>> m_bins;
	std::unordered_map<std::string, int> m_regionToBin;
};

class ChoroplethPainting : public renderer::GeometryPainting {
  public:
	Choropleth& m_choropleth;
	std::vector<Color> m_colors;
	Color m_noDataColor;
	bool m_drawLabels;

	template <class InputIterator>
	ChoroplethPainting(Choropleth& choropleth, InputIterator beginColors, InputIterator endColors,
	                   bool drawLabels = false, Color noDataColor = Color(200, 200, 200))
	    : m_choropleth(choropleth), m_colors(beginColors, endColors),
	      m_noDataColor(noDataColor), m_drawLabels(drawLabels) {}

	void paint(renderer::GeometryRenderer& renderer) const override;
};
}

#endif //CARTOCROW_CHOROPLETH_H
