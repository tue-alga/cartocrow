/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

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
*/

#include "region_arrangement.h"

#include <CGAL/Arr_overlay_2.h>
#include <CGAL/Surface_sweep_2/Arr_default_overlay_traits_base.h>
#include <CGAL/number_utils.h>

#include <future>

namespace cartocrow {

namespace detail {
struct RegionOverlayTraits
    : public CGAL::_Arr_default_overlay_traits_base<RegionArrangement, PolygonSet<Exact>::Arrangement_2,
                                                    RegionArrangement> {

	RegionOverlayTraits(std::string newId) : m_newId(newId) {}

	virtual void create_face(Face_handle_A f1, Face_handle_B f2, Face_handle_R f) {
		if (f2->contained()) {
			std::string id = f1->data();
			if (f1->data() != "") {
				f->set_data(f1->data());
			} else {
				f->set_data(m_newId);
			}
		} else {
			f->set_data(f1->data());
		}
	}

  private:
	std::string m_newId;
};

struct PickRegion {
    std::string operator()(const std::string& region1, const std::string& region2) const {
        if (!region1.empty() && !region2.empty()) {
            std::cerr << "Overlapping regions! " << region1 << " and " << region2 << std::endl;
            return region1;
        }
        if (region1.empty()) {
            return region2;
        } else {
            return region1;
        }
    }
};

using RegionPickTraits = CGAL::Arr_face_overlay_traits<RegionArrangement, RegionArrangement,
                RegionArrangement, PickRegion>;
} // namespace detail

RegionArrangement regionMapToArrangement(const RegionMap& map) {
	RegionArrangement arrangement;

	for (const auto& [id, region] : map) {
		RegionArrangement result;
		detail::RegionOverlayTraits overlayTraits(id);
		CGAL::overlay(arrangement, region.shape.arrangement(), result, overlayTraits);
		arrangement = result;
	}

	return arrangement;
}

RegionArrangement regionMapToArrangementParallel(const RegionMap& map) {
    std::vector<std::future<RegionArrangement>> results;

    int nThreads = 16;
    std::vector<std::string> keys;
    for (auto& [key, _] : map) {
        keys.push_back(key);
    }
    int n = keys.size();
    double step = n / static_cast<double>(nThreads);

    auto task = [&keys, &map](int iStart, int iEnd) {
        RegionArrangement arrangement;

        for (int i = iStart; i != iEnd; ++i) {
            auto& id = keys[i];
            auto& region = map.at(id);
            RegionArrangement result;
            detail::RegionOverlayTraits overlayTraits(id);
            CGAL::overlay(arrangement, region.shape.arrangement(), result, overlayTraits);
            arrangement = result;
        }

        return arrangement;
    };

    for (int i = 0; i < n / step; ++i) {
        int iStart = std::ceil(i * step);
        int iEnd = std::ceil((i + 1) * step);
        results.push_back(std::async(task, iStart, iEnd));
    }

    RegionArrangement arrangement;
    for (auto& futureResult : results) {
        RegionArrangement partialResult = futureResult.get();
        RegionArrangement result;
        detail::RegionPickTraits overlayTraits;
        CGAL::overlay(arrangement, partialResult, result, overlayTraits);
        arrangement = result;
    }

    return arrangement;
}

} // namespace cartocrow
