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
    : m_r(0), m_firstInterval(SweepInterval(SweepInterval::Type::REACHABLE)) {}

Number<Inexact> SweepCircle::r() {
	return m_r;
}

void SweepCircle::grow(Number<Inexact> r) {
	m_r = r;

	// TODO reinsert out-of-bounds elements (φ < 0 || φ >= 2π), make sure to
	// maintain the right order and update the first / last intervals!
}

bool SweepCircle::isValid() const {
	Number<Inexact> previousPhi = 0;
	for (auto& edge : m_edges) {
		Number<Inexact> phi = edge.first.phiForR(m_r);
		if (phi < previousPhi) {
			return false;
		}
		previousPhi = phi;
	}
	return true;
}

void SweepCircle::print() const {
	auto printIntervalType = [](const SweepInterval& interval) {
		if (interval.type() == SweepInterval::Type::SHADOW) {
			std::cout << "shadow";
		} else if (interval.type() == SweepInterval::Type::REACHABLE) {
			std::cout << "reachable";
		} else if (interval.type() == SweepInterval::Type::OBSTACLE) {
			std::cout << "obstacle";
		}
	};
	std::cout << "sweep circle at r = " << m_r << ": |0π| ";
	printIntervalType(m_firstInterval);
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
	std::cout << " |2π|" << std::endl;
}

std::size_t SweepCircle::intervalCount() const {
	return m_edges.size() + 1;
}

SweepInterval* SweepCircle::intervalAt(Number<Inexact> phi) {
	if (m_edges.empty()) {
		return &m_firstInterval;
	}
	auto edgeIterator = m_edges.lower_bound(phi);
	if (edgeIterator == m_edges.end()) {
		// next interval of the last edge
		return &(--m_edges.end())->second->m_nextInterval;
	}
	std::shared_ptr<SweepEdge> edge = edgeIterator->second;
	return edge->previousInterval();
}

std::shared_ptr<SweepEdge> SweepCircle::edgeAt(Number<Inexact> phi) {
	auto edgeIterator = m_edges.find(phi);
	if (edgeIterator == m_edges.end()) {
		return nullptr;
	}
	return edgeIterator->second;
}

SweepCircle::SplitResult SweepCircle::splitFromEdge(std::shared_ptr<SweepEdge> oldEdge,
                                                    std::shared_ptr<SweepEdge> newRightEdge,
                                                    std::shared_ptr<SweepEdge> newLeftEdge) {
	// sanity checks: the new edge shapes start where the old edge ends
	assert(oldEdge->shape().farR() == m_r);
	assert(newRightEdge->shape().nearR() == m_r);
	assert(newLeftEdge->shape().nearR() == m_r);
	assert(oldEdge->shape().phiForR(m_r) == newRightEdge->shape().phiForR(m_r));
	assert(oldEdge->shape().phiForR(m_r) == newLeftEdge->shape().phiForR(m_r));
	SweepInterval* previousInterval = oldEdge->previousInterval();
	SweepInterval* nextInterval = oldEdge->nextInterval();

	m_edges.erase(oldEdge->shape());
	m_edges.insert(std::make_pair(newRightEdge->shape(), newRightEdge));
	m_edges.insert(std::make_pair(newLeftEdge->shape(), newLeftEdge));

	previousInterval->m_nextBoundary = newRightEdge.get();
	newRightEdge->m_previousInterval = previousInterval;
	newRightEdge->m_nextInterval =
	    SweepInterval(previousInterval->type(), newRightEdge.get(), newLeftEdge.get());
	nextInterval->m_previousBoundary = newLeftEdge.get();
	newLeftEdge->m_nextInterval = *nextInterval;
	newLeftEdge->m_nextInterval.m_previousBoundary = newLeftEdge.get();

	return SplitResult{newRightEdge->previousInterval(), newRightEdge->nextInterval(),
	                   newLeftEdge->nextInterval()};
}

SweepCircle::SplitResult SweepCircle::splitFromInterval(std::shared_ptr<SweepEdge> newRightEdge,
                                                        std::shared_ptr<SweepEdge> newLeftEdge) {
	// sanity checks: the edges to split start at the same phi
	assert(newLeftEdge->shape().nearR() == m_r);
	assert(newLeftEdge->shape().nearR() == newRightEdge->shape().nearR());
	Number<Inexact> phi = newLeftEdge->shape().phiForR(m_r);
	assert(phi == newRightEdge->shape().phiForR(m_r));
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
		m_firstInterval = SweepInterval(interval->type(), nullptr, newRightEdge.get());
		newRightEdge->m_previousInterval = &m_firstInterval;
	}

	newRightEdge->m_nextInterval =
	    SweepInterval(interval->type(), newRightEdge.get(), newLeftEdge.get());
	newLeftEdge->m_previousInterval = &newRightEdge->m_nextInterval;

	newLeftEdge->m_nextInterval = SweepInterval(interval->type(), newLeftEdge.get(), nextEdge);
	if (nextEdge) {
		nextEdge->m_previousInterval = &newLeftEdge->m_nextInterval;
	}

	return SplitResult{newRightEdge->previousInterval(), newRightEdge->nextInterval(),
	                   newLeftEdge->nextInterval()};
}

SweepCircle::SwitchResult SweepCircle::switchEdge(std::shared_ptr<SweepEdge> e,
                                                  std::shared_ptr<SweepEdge> newEdge) {
	// sanity checks: the new edge shape starts where the current edge ends
	assert(e->shape().farR() == m_r);
	assert(newEdge->shape().nearR() == m_r);
	assert(newEdge->shape().phiForR(m_r) == e->m_shape.phiForR(m_r));
	SweepEdge* previousEdge = e->previousEdge();
	SweepEdge* nextEdge = e->nextEdge();

	if (previousEdge) {
		previousEdge->m_nextInterval =
		    SweepInterval(previousEdge->m_nextInterval.type(), previousEdge, newEdge.get());
		newEdge->m_previousInterval = &previousEdge->m_nextInterval;
	} else {
		m_firstInterval = SweepInterval(m_firstInterval.type(), nullptr, newEdge.get());
		newEdge->m_previousInterval = &m_firstInterval;
	}
	newEdge->m_nextInterval = SweepInterval(e->m_nextInterval.type(), newEdge.get(), nextEdge);

	m_edges.erase(e->shape());
	m_edges.insert(std::make_pair(newEdge->shape(), newEdge));

	if (nextEdge) {
		nextEdge->m_previousInterval = &newEdge->m_nextInterval;
	}

	return SwitchResult{newEdge->previousInterval(), newEdge->nextInterval()};
}

SweepCircle::SwitchResult SweepCircle::mergeToEdge(std::shared_ptr<SweepEdge> rightEdge,
                                                   std::shared_ptr<SweepEdge> leftEdge,
                                                   std::shared_ptr<SweepEdge> newEdge) {
	// sanity checks: the new edge shape starts where the old edges end
	assert(leftEdge->shape().farR() == m_r);
	assert(rightEdge->shape().farR() == m_r);
	assert(newEdge->shape().nearR() == m_r);
	assert(leftEdge->shape().phiForR(m_r) == newEdge->shape().phiForR(m_r));
	assert(rightEdge->shape().phiForR(m_r) == newEdge->shape().phiForR(m_r));

	SweepEdge* previousEdge = rightEdge->previousEdge();
	SweepEdge* nextEdge = leftEdge->nextEdge();
	if (previousEdge) {
		previousEdge->m_nextInterval =
		    SweepInterval(rightEdge->m_previousInterval->type(), previousEdge, newEdge.get());
	} else {
		m_firstInterval =
		    SweepInterval(rightEdge->m_previousInterval->type(), nullptr, newEdge.get());
	}
	newEdge->m_nextInterval = SweepInterval(leftEdge->m_nextInterval.type(), newEdge.get(), nextEdge);

	if (nextEdge) {
		nextEdge->m_previousInterval = &newEdge->m_nextInterval;
	}

	// this erases both edges as they have the same phi
	m_edges.erase(leftEdge->shape());
	m_edges.insert(std::make_pair(newEdge->shape(), newEdge));

	return SwitchResult{newEdge->previousInterval(), newEdge->nextInterval()};
}

SweepCircle::MergeResult SweepCircle::mergeToInterval(std::shared_ptr<SweepEdge> rightEdge,
                                                      std::shared_ptr<SweepEdge> leftEdge) {
	// sanity checks: the edges to merge end at the same phi
	assert(leftEdge->shape().farR() == m_r);
	assert(rightEdge->shape().farR() == m_r);
	assert(leftEdge->shape().phiForR(m_r) == rightEdge->shape().phiForR(m_r));

	SweepEdge* previousEdge = rightEdge->previousEdge();
	SweepEdge* nextEdge = leftEdge->nextEdge();
	if (previousEdge) {
		previousEdge->m_nextInterval =
		    SweepInterval(rightEdge->m_nextInterval.type(), previousEdge, nextEdge);
	} else {
		m_firstInterval = SweepInterval(rightEdge->m_nextInterval.type(), nullptr, nextEdge);
	}
	if (nextEdge) {
		nextEdge->m_previousInterval = &previousEdge->m_nextInterval;
	}

	// this erases both edges as they have the same phi
	m_edges.erase(leftEdge->shape());

	if (previousEdge) {
		return MergeResult{previousEdge->nextInterval()};
	} else {
		return MergeResult{&m_firstInterval};
	}
}

const SweepCircle::EdgeMap& SweepCircle::edges() const {
	return m_edges;
}

const SweepInterval& SweepCircle::firstInterval() const {
	return m_firstInterval;
}

} // namespace cartocrow::flow_map
