/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3f of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "sweep_circle.h"

#include "polar_segment.h"
#include "spiral_segment.h"

namespace cartocrow::flow_map {

SweepCircle::SweepCircle()
    : m_r(0), m_onlyInterval(SweepInterval(SweepInterval::Type::REACHABLE)) {}

Number<Inexact> SweepCircle::r() {
	return m_r;
}

void SweepCircle::grow(Number<Inexact> r) {
	Number<Inexact> previousR = m_r;
	m_r = r;

	if (m_edges.empty()) {
		return;
	}

	// reinsert edges that moved counter-clockwise over the φ = π ray
	std::vector<EdgeMap::node_type> toReinsert;
	for (auto e = --m_edges.end(); e != m_edges.begin(); --e) {
		auto beforeGrowing = e->first.evalForR(previousR);
		auto afterGrowing = e->first.evalForR(r);
		if (beforeGrowing.phi() > M_PI / 2 && afterGrowing.phi() < 0) {
			/*std::cout << "counter-clockwise wraparound at φ = " << afterGrowing.phi() / M_PI << "π"
			          << std::endl;*/
			toReinsert.push_back(m_edges.extract(e));
		} else {
			break;
		}
	}
	for (auto& node : toReinsert) {
		m_edges.insert(std::move(node));
	}

	// reinsert edges that moved clockwise over the φ = π ray
	toReinsert.clear();
	for (auto e = m_edges.begin(); e != --m_edges.end(); ++e) {
		auto beforeGrowing = e->first.evalForR(previousR);
		auto afterGrowing = e->first.evalForR(r);
		if (beforeGrowing.phi() < -M_PI / 2 && afterGrowing.phi() > 0) {
			/*std::cout << "clockwise wraparound at φ = " << afterGrowing.phi() / M_PI << "π"
			          << std::endl;*/
			toReinsert.push_back(m_edges.extract(e));
		} else {
			break;
		}
	}
	for (auto& node : toReinsert) {
		m_edges.insert(std::move(node));
	}
}

bool SweepCircle::isValid() const {
	Number<Inexact> previousPhi = -M_PI;
	bool valid = true;
	if (m_edges.empty()) {
		if (!m_onlyInterval.has_value()) {
			std::cout << "sweep circle is invalid because it doesn't have edges, but onlyInterval "
			             "isn't set"
			          << std::endl;
		}
	} else {
		if (m_onlyInterval.has_value()) {
			std::cout
			    << "sweep circle is invalid because it has edges, but onlyInterval is still set"
			    << std::endl;
		}
		int edgeId = 0;
		for (auto edge = m_edges.begin(); edge != m_edges.end(); ++edge) {
			Number<Inexact> phi = edge->first.phiForR(m_r);
			if (phi < previousPhi) {
				std::cout << "sweep circle is invalid because edge " << edgeId
				          << " (at φ = " << phi / M_PI
				          << "π) is ordered after the edge at φ = " << previousPhi / M_PI << "π"
				          << std::endl;
				valid = false;
			}
			previousPhi = phi;

			if (edge->second->m_previousInterval == nullptr) {
				std::cout << "sweep circle is invalid because edge " << edgeId
				          << " has nullptr as its previous interval" << std::endl;
				valid = false;
			} else if (edge->second->m_previousInterval->m_nextBoundary != edge->second.get()) {
				std::cout << "sweep circle is invalid because the next boundary of the previous "
				             "interval of edge "
				          << edgeId << " is not equal to edge " << edgeId << std::endl;
				valid = false;
			}
			if (edge->second->m_nextInterval.m_previousBoundary != edge->second.get()) {
				std::cout << "sweep circle is invalid because the previous boundary of the next "
				             "interval of edge "
				          << edgeId << " is not equal to edge " << edgeId << std::endl;
				valid = false;
			}
			if (std::next(edge) == m_edges.end()) {
				if (edge->second->m_nextInterval.m_nextBoundary != m_edges.begin()->second.get()) {
					std::cout << "sweep circle is invalid because the next boundary of the next "
					             "interval of edge "
					          << edgeId << " is not equal to edge 0" << std::endl;
					valid = false;
				}
			} else {
				if (edge->second->m_nextInterval.m_nextBoundary != std::next(edge)->second.get()) {
					std::cout << "sweep circle is invalid because the next boundary of the next "
					             "interval of edge "
					          << edgeId << " is not equal to edge " << edgeId + 1 << std::endl;
					valid = false;
				}
			}
			edgeId++;
		}
	}
	return valid;
}

void SweepCircle::print() const {
	auto printIntervalType = [](const SweepInterval& interval) {
		if (interval.type() == SweepInterval::Type::SHADOW) {
			std::cout << "\033[1mshadow\033[0m";
		} else if (interval.type() == SweepInterval::Type::REACHABLE) {
			std::cout << "\033[1;32mreachable\033[0m";
		} else if (interval.type() == SweepInterval::Type::OBSTACLE) {
			std::cout << "\033[1;31mobstacle\033[0m";
		}
	};
	std::cout << "sweep circle at \033[1mr = " << m_r << "\033[0m: ← ";
	if (m_edges.empty()) {
		printIntervalType(*m_onlyInterval);
	} else {
		printIntervalType(*m_edges.begin()->second->previousInterval());
		for (auto& edge : m_edges) {
			if (edge.first.type() == SweepEdgeShape::Type::SEGMENT) {
				std::cout << " |" << edge.first.phiForR(m_r) / M_PI << "π| ";
			} else if (edge.first.type() == SweepEdgeShape::Type::LEFT_SPIRAL) {
				std::cout << " )" << edge.first.phiForR(m_r) / M_PI << "π) ";
			} else if (edge.first.type() == SweepEdgeShape::Type::RIGHT_SPIRAL) {
				std::cout << " (" << edge.first.phiForR(m_r) / M_PI << "π( ";
			}
			printIntervalType(*edge.second->nextInterval());
		}
	}
	std::cout << " →" << std::endl;
}

std::size_t SweepCircle::intervalCount() const {
	return std::max(std::size_t(1), m_edges.size());
}

SweepInterval* SweepCircle::intervalAt(Number<Inexact> phi) {
	if (m_edges.empty()) {
		return &*m_onlyInterval;
	}
	auto edgeIterator = m_edges.lower_bound(phi);
	if (edgeIterator == m_edges.end()) {
		// next interval of the last edge
		return &(--m_edges.end())->second->m_nextInterval;
	}
	std::shared_ptr<SweepEdge> edge = edgeIterator->second;
	return edge->previousInterval();
}

SweepCircle::EdgeMap::iterator SweepCircle::begin() {
	return m_edges.begin();
}

std::pair<SweepCircle::EdgeMap::iterator, SweepCircle::EdgeMap::iterator>
SweepCircle::edgesAt(Number<Inexact> phi) {
	return m_edges.equal_range(phi);
}

SweepCircle::EdgeMap::iterator SweepCircle::end() {
	return m_edges.end();
}

SweepCircle::SplitResult SweepCircle::splitFromEdge(SweepEdge& oldEdge,
                                                    std::shared_ptr<SweepEdge> newRightEdge,
                                                    std::shared_ptr<SweepEdge> newLeftEdge) {
	SweepEdge* previousEdge = oldEdge.previousEdge();
	SweepEdge* nextEdge = oldEdge.nextEdge();
	SweepInterval nextInterval = oldEdge.m_nextInterval;

	m_edges.erase(oldEdge.shape());
	m_edges.insert(std::make_pair(newRightEdge->shape(), newRightEdge));
	m_edges.insert(std::make_pair(newLeftEdge->shape(), newLeftEdge));

	if (previousEdge) {
		previousEdge->m_nextInterval.m_nextBoundary = newRightEdge.get();
		newRightEdge->m_previousInterval = &previousEdge->m_nextInterval;
	}

	newRightEdge->m_nextInterval =
	    SweepInterval(nextInterval.type(), newRightEdge.get(), newLeftEdge.get());
	newLeftEdge->m_previousInterval = &newRightEdge->m_nextInterval;

	newLeftEdge->m_nextInterval = SweepInterval(nextInterval.type(), newLeftEdge.get(), nextEdge);
	if (nextEdge) {
		nextEdge->m_previousInterval = &newLeftEdge->m_nextInterval;
	}

	return SplitResult{newRightEdge->previousInterval(), newRightEdge->nextInterval(),
	                   newLeftEdge->nextInterval()};
}

SweepCircle::ThreeWaySplitResult
SweepCircle::splitFromInterval(std::shared_ptr<SweepEdge> newRightEdge,
                               std::shared_ptr<SweepEdge> newMiddleEdge,
                               std::shared_ptr<SweepEdge> newLeftEdge) {
	Number<Inexact> phi = newLeftEdge->shape().phiForR(m_r);

	SweepInterval* interval = intervalAt(phi);
	SweepEdge* previousEdge = interval->previousBoundary();
	SweepEdge* nextEdge = interval->nextBoundary();

	m_edges.insert(std::make_pair(newRightEdge->shape(), newRightEdge));
	m_edges.insert(std::make_pair(newMiddleEdge->shape(), newMiddleEdge));
	m_edges.insert(std::make_pair(newLeftEdge->shape(), newLeftEdge));

	if (previousEdge) {
		previousEdge->m_nextInterval =
		    SweepInterval(interval->type(), previousEdge, newRightEdge.get());
		newRightEdge->m_previousInterval = &previousEdge->m_nextInterval;
	} else {
		newRightEdge->m_previousInterval = &newLeftEdge->m_nextInterval;
	}

	newRightEdge->m_nextInterval =
	    SweepInterval(interval->type(), newRightEdge.get(), newMiddleEdge.get());
	newMiddleEdge->m_previousInterval = &newRightEdge->m_nextInterval;

	newMiddleEdge->m_nextInterval =
	    SweepInterval(interval->type(), newMiddleEdge.get(), newLeftEdge.get());
	newLeftEdge->m_previousInterval = &newMiddleEdge->m_nextInterval;

	newLeftEdge->m_nextInterval = SweepInterval(interval->type(), newLeftEdge.get(), nextEdge);
	if (nextEdge) {
		nextEdge->m_previousInterval = &newLeftEdge->m_nextInterval;
	} else {
		newLeftEdge->m_nextInterval =
		    SweepInterval(interval->type(), newLeftEdge.get(), newRightEdge.get());
	}

	m_onlyInterval = std::nullopt;

	return ThreeWaySplitResult{newRightEdge->previousInterval(), newRightEdge->nextInterval(),
	                           newMiddleEdge->nextInterval(), newLeftEdge->nextInterval()};
}

SweepCircle::SplitResult SweepCircle::splitFromInterval(std::shared_ptr<SweepEdge> newRightEdge,
                                                        std::shared_ptr<SweepEdge> newLeftEdge) {
	Number<Inexact> phi = newLeftEdge->shape().phiForR(m_r);

	SweepInterval* interval = intervalAt(phi);
	SweepEdge* previousEdge = interval->previousBoundary();
	SweepEdge* nextEdge = interval->nextBoundary();

	m_edges.insert(std::make_pair(newRightEdge->shape(), newRightEdge));
	m_edges.insert(std::make_pair(newLeftEdge->shape(), newLeftEdge));

	if (previousEdge) {
		previousEdge->m_nextInterval =
		    SweepInterval(interval->type(), previousEdge, newRightEdge.get());
		newRightEdge->m_previousInterval = &previousEdge->m_nextInterval;
	} else {
		newRightEdge->m_previousInterval = &newLeftEdge->m_nextInterval;
	}

	newRightEdge->m_nextInterval =
	    SweepInterval(interval->type(), newRightEdge.get(), newLeftEdge.get());
	newLeftEdge->m_previousInterval = &newRightEdge->m_nextInterval;

	if (nextEdge) {
		newLeftEdge->m_nextInterval = SweepInterval(interval->type(), newLeftEdge.get(), nextEdge);
		nextEdge->m_previousInterval = &newLeftEdge->m_nextInterval;
	} else {
		newLeftEdge->m_nextInterval =
		    SweepInterval(interval->type(), newLeftEdge.get(), newRightEdge.get());
	}

	m_onlyInterval = std::nullopt;

	return SplitResult{newRightEdge->previousInterval(), newRightEdge->nextInterval(),
	                   newLeftEdge->nextInterval()};
}

SweepCircle::SwitchResult SweepCircle::switchEdge(SweepEdge& e, std::shared_ptr<SweepEdge> newEdge) {
	SweepEdge* previousEdge = e.previousEdge();
	SweepEdge* nextEdge = e.nextEdge();

	previousEdge->m_nextInterval =
	    SweepInterval(previousEdge->m_nextInterval.type(), previousEdge, newEdge.get());
	newEdge->m_previousInterval = &previousEdge->m_nextInterval;
	newEdge->m_nextInterval = SweepInterval(e.m_nextInterval.type(), newEdge.get(), nextEdge);

	m_edges.erase(e.shape());
	m_edges.insert(std::make_pair(newEdge->shape(), newEdge));

	nextEdge->m_previousInterval = &newEdge->m_nextInterval;

	return SwitchResult{newEdge->previousInterval(), newEdge->nextInterval()};
}

SweepCircle::SwitchResult SweepCircle::mergeToEdge(SweepEdge& rightEdge, SweepEdge& leftEdge,
                                                   std::shared_ptr<SweepEdge> newEdge) {
	SweepEdge* previousEdge = rightEdge.previousEdge();
	SweepEdge* nextEdge = leftEdge.nextEdge();
	previousEdge->m_nextInterval =
	    SweepInterval(rightEdge.m_previousInterval->type(), previousEdge, newEdge.get());
	newEdge->m_previousInterval = &previousEdge->m_nextInterval;
	newEdge->m_nextInterval = SweepInterval(leftEdge.m_nextInterval.type(), newEdge.get(), nextEdge);
	nextEdge->m_previousInterval = &newEdge->m_nextInterval;

	m_edges.erase(rightEdge.shape());
	m_edges.erase(leftEdge.shape());
	m_edges.insert(std::make_pair(newEdge->shape(), newEdge));

	return SwitchResult{newEdge->previousInterval(), newEdge->nextInterval()};
}

SweepCircle::MergeResult SweepCircle::mergeToInterval(SweepEdge& rightEdge, SweepEdge& leftEdge) {
	SweepEdge* previousEdge = rightEdge.previousEdge();
	SweepEdge* nextEdge = leftEdge.nextEdge();
	SweepInterval interval = SweepInterval(leftEdge.m_nextInterval.type(), nullptr, nullptr);
	previousEdge->m_nextInterval =
	    SweepInterval(rightEdge.m_nextInterval.type(), previousEdge, nextEdge);
	if (nextEdge) {
		nextEdge->m_previousInterval = &previousEdge->m_nextInterval;
	}

	m_edges.erase(rightEdge.shape());
	m_edges.erase(leftEdge.shape());

	if (m_edges.empty()) {
		m_onlyInterval = interval;
		return MergeResult{&*m_onlyInterval};
	} else {
		return MergeResult{previousEdge->nextInterval()};
	}
}

const SweepCircle::EdgeMap& SweepCircle::edges() const {
	return m_edges;
}

} // namespace cartocrow::flow_map
