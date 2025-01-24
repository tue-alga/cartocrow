#ifndef CARTOCROW_CHOROPLETH_H
#define CARTOCROW_CHOROPLETH_H

#include <utility>
#include "../renderer/geometry_painting.h"
#include "../core/region_arrangement.h"
#include "../core/arrangement_helpers.h"
#include "natural_breaks.h"

#include <CGAL/Aff_transformation_2.h>

namespace cartocrow::chorematic_map {
/// A choropleth map. It is a \ref RegionArrangement where double values are associated with each region.
/// Thresholds that define classes can be passed to the constructor.
/// If no thresholds are passed, the Fisher-Jenks natural breaks algorithm is used to compute natural thresholds.
///
/// This class represents only an abstract choropleth; to draw one, see \ref ChoroplethPainting.
class Choropleth {
  public:
	std::shared_ptr<RegionArrangement> m_arr;
	std::shared_ptr<std::unordered_map<std::string, double>> m_data;

    void naturalBreaks(int nBins) {
        m_thresholds.clear();
        std::vector<double> values;
        for (auto& [_, value] : *m_data) {
            values.push_back(value);
        }
        natural_breaks(values.begin(), values.end(), std::back_inserter(m_thresholds), nBins);
    }

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

    Choropleth(std::shared_ptr<RegionArrangement> arr,
               std::shared_ptr<std::unordered_map<std::string, double>> data,
               int nBins) :
            m_arr(std::move(arr)), m_data(std::move(data)) {
        naturalBreaks(nBins);
        rebin();
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

	std::vector<double>& getThresholds() {
		return m_thresholds;
	}

	std::vector<double> getIntervals() {
		std::optional<double> min;
		std::optional<double> max;
		for (auto& [region, value] : *m_data) {
			if (!min.has_value() || value < *min) {
				min = value;
			}
			if (!max.has_value() || value > *max) {
				max = value;
			}
		}
		std::vector<double> intervals({*min});
		for (const auto& t : getThresholds()) {
			intervals.push_back(t);
		}
		intervals.push_back(*max);
		return intervals;
	}

    int numberOfBins() const {
        return m_bins.size();
    }

    std::vector<Number<Exact>> binAreas() const {
        std::vector<Number<Exact>> areas;
        for (int i = 0; i < m_bins.size(); ++i) {
            areas.emplace_back(0);
        }
        for (auto fit = m_arr->faces_begin(); fit != m_arr->faces_end(); ++fit) {
            auto pwh = face_to_polygon_with_holes<Exact>(fit);
            auto& region = fit->data();
            if (!m_regionToBin.contains(region)) continue;
            auto& bin = m_regionToBin.at(region);
            areas[bin] += abs(pwh.outer_boundary().area());
            for (auto& hole : pwh.holes()) {
                areas[bin] -= abs(hole.area());
            }
        }
        return areas;
    }

  private:
	std::vector<double> m_thresholds;
	std::vector<std::vector<std::string>> m_bins;
	std::unordered_map<std::string, int> m_regionToBin;
};

/// Draws a \ref Choropleth. One should pass a color for each bin of the choropleth.
/// Drawing style can be configured via \ref ChoroplethPainting::Options.
class ChoroplethPainting : public renderer::GeometryPainting {
  public:
	Choropleth& m_choropleth;
	std::vector<Color> m_colors;

	struct Options {
		bool drawLabels = false;
		Color noDataColor = Color(200, 200, 200);
		CGAL::Aff_transformation_2<Inexact> transformation = CGAL::IDENTITY;
		double strokeWidth = 1.0;
		Color strokeColor = Color(0, 0, 0);
	};

	Options m_options;

	// Needed because bug in clang gcc:
	// https://stackoverflow.com/questions/53408962/try-to-understand-compiler-error-message-default-member-initializer-required-be
	static Options defaultOptions() {
		return {};
	}

	template <class InputIterator>
	ChoroplethPainting(Choropleth& choropleth, InputIterator beginColors, InputIterator endColors, Options options = defaultOptions())
	    : m_choropleth(choropleth), m_colors(beginColors, endColors), m_options(options) {}

	template <class InputIterator>
	void setColors(InputIterator begin, InputIterator end) {
		m_colors.clear();
		m_colors.resize(0);
		std::copy(begin, end, std::back_inserter(m_colors));
	}

	void paint(renderer::GeometryRenderer& renderer) const override;
};
}

#endif //CARTOCROW_CHOROPLETH_H
