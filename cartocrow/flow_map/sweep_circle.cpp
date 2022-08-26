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
#include <stdexcept>

namespace cartocrow::flow_map {

SweepInterval::SweepInterval(Type type)
    : m_type(type), m_nextBoundary(nullptr), m_previousBoundary(nullptr) {}

void SweepInterval::setNextBoundary(SweepEdge* nextBoundary) {
	m_nextBoundary = nextBoundary;
}

SweepEdge* SweepInterval::nextBoundary() const {
	return m_nextBoundary;
}

void SweepInterval::setPreviousBoundary(SweepEdge* previousBoundary) {
	m_previousBoundary = previousBoundary;
}

SweepEdge* SweepInterval::previousBoundary() const {
	return m_previousBoundary;
}

void SweepInterval::setType(Type type) {
	m_type = type;
}

SweepInterval::Type SweepInterval::type() const {
	return m_type;
}

SweepEdgeShape::SweepEdgeShape(Type type, PolarPoint p1, PolarPoint p2)
    : m_type(type), m_p1(p1), m_p2(p2) {
	if (m_p2.r() < m_p1.r()) {
		std::swap(m_p1, m_p2);
	}
}

Number<Inexact> SweepEdgeShape::nearR() const {
	return m_p1.r();
}

Number<Inexact> SweepEdgeShape::farR() const {
	return m_p2.r();
}

Number<Inexact> SweepEdgeShape::phiForR(Number<Inexact> r) const {
	assert(r >= nearR() && r <= farR()); // trying to compute phi for out-of-bounds r

	// for robustness, it is important that if we are exactly at the beginning
	// or end of the edge, we return the exact phi of its begin or end point
	if (r == nearR()) {
		return m_p1.phi();
	}
	if (r == farR()) {
		return m_p2.phi();
	}

	if (m_type == Type::SEGMENT) {
		PolarSegment s(m_p1, m_p2);
		std::array<Number<Inexact>, 2> phis;
		int phiCount = s.collectPhi(r, &phis[0]);
		assert(phiCount != 0); // line segment did not contain the given phi
		assert(phiCount == 1); // line segment contained two of the given phis
		return phis[0];
	} else {
		SpiralSegment s(m_p1, m_p2);
		return s.supportingSpiral().phiForR(r);
	}
}

SweepEdge::SweepEdge(SweepEdgeShape shape) : m_shape(shape) {}

const SweepEdgeShape& SweepEdge::shape() const {
	return m_shape;
}

SweepInterval* SweepEdge::nextInterval() const {
	return m_nextInterval.get();
}

SweepInterval* SweepEdge::previousInterval() const {
	return m_previousInterval.get();
}

SweepCircle::SweepCircle() : m_r(0) {
	m_firstInterval = std::make_shared<SweepInterval>(SweepInterval::Type::REACHABLE);
	m_lastInterval = m_firstInterval;
}

Number<Inexact> SweepCircle::r() {
	return m_r;
}

void SweepCircle::grow(Number<Inexact> r) {
	m_r = r;
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
	auto printIntervalType = [](const SweepInterval* interval) {
		if (interval->type() == SweepInterval::Type::SHADOW) {
			std::cout << "shadow";
		} else if (interval->type() == SweepInterval::Type::REACHABLE) {
			std::cout << "reachable";
		} else if (interval->type() == SweepInterval::Type::OBSTACLE) {
			std::cout << "obstacle";
		}
	};
	std::cout << "sweep circle at r = " << m_r << ": [0π] ";
	printIntervalType(m_firstInterval.get());
	for (auto& edge : m_edges) {
		std::cout << " [" << edge.first.phiForR(m_r) / M_PI << "π] ";
		printIntervalType(edge.second.nextInterval());
	}
	std::cout << " [2π]" << std::endl;
}

std::size_t SweepCircle::intervalCount() const {
	return m_edges.size() + 1;
}

SweepInterval* SweepCircle::intervalAt(Number<Inexact> phi) {
	if (m_edges.empty()) {
		return m_firstInterval.get();
	}
	auto edgeIterator = m_edges.upper_bound(phi);
	if (edgeIterator == m_edges.end()) {
		return m_lastInterval.get();
	}
	SweepEdge& edge = edgeIterator->second;
	return edge.previousInterval();
}

SweepEdge* SweepCircle::edgeAt(Number<Inexact> phi) {
	// TODO
	return nullptr;
}

SweepCircle::SplitResult SweepCircle::splitFromEdge(SweepEdge* e, SweepEdgeShape newLeftEdgeShape,
                                                    SweepEdgeShape newRightEdgeShape) {
	// TODO
	assert(false);
}

SweepCircle::SplitResult SweepCircle::splitFromInterval(SweepInterval* interval,
                                                        SweepEdgeShape newRightEdgeShape,
                                                        SweepEdgeShape newLeftEdgeShape) {
	// sanity checks: the new vertex falls within the interval given
	assert(newLeftEdgeShape.nearR() == m_r);
	assert(newLeftEdgeShape.nearR() == newRightEdgeShape.nearR());
	assert(newLeftEdgeShape.phiForR(m_r) == newRightEdgeShape.phiForR(m_r));
	if (interval->previousBoundary()) {
		assert(newLeftEdgeShape.phiForR(m_r) > interval->previousBoundary()->m_shape.phiForR(m_r));
	}
	if (interval->nextBoundary()) {
		assert(newLeftEdgeShape.phiForR(m_r) < interval->nextBoundary()->m_shape.phiForR(m_r));
	}

	SweepEdge newRightEdge(newRightEdgeShape);
	SweepEdge newLeftEdge(newLeftEdgeShape);
	auto& insertedRightEdge =
	    (m_edges.insert(std::make_pair(newRightEdgeShape, newRightEdge)))->second;
	auto& insertedLeftEdge = (m_edges.insert(std::make_pair(newLeftEdgeShape, newLeftEdge)))->second;

	auto rightInterval = std::make_shared<SweepInterval>(*interval);
	rightInterval->setPreviousBoundary(interval->previousBoundary());
	if (interval->previousBoundary()) {
		interval->previousBoundary()->m_nextInterval = rightInterval;
	} else {
		m_firstInterval = rightInterval;
	}
	rightInterval->setNextBoundary(&insertedRightEdge);
	insertedRightEdge.m_previousInterval = rightInterval;

	auto middleInterval = std::make_shared<SweepInterval>(*interval);
	insertedRightEdge.m_nextInterval = middleInterval;
	middleInterval->setPreviousBoundary(&insertedRightEdge);
	middleInterval->setNextBoundary(&insertedLeftEdge);
	insertedLeftEdge.m_previousInterval = middleInterval;

	auto leftInterval = std::make_shared<SweepInterval>(*interval);
	insertedLeftEdge.m_nextInterval = leftInterval;
	leftInterval->setPreviousBoundary(&insertedLeftEdge);
	leftInterval->setNextBoundary(interval->nextBoundary());
	if (interval->nextBoundary()) {
		interval->nextBoundary()->m_previousInterval = leftInterval;
	} else {
		m_lastInterval = leftInterval;
	}

	return SplitResult{rightInterval.get(), &insertedRightEdge, middleInterval.get(),
	                   &insertedLeftEdge, leftInterval.get()};
}

SweepCircle::SwitchResult SweepCircle::switchEdge(SweepEdge* e, SweepEdgeShape newEdgeShape) {
	// sanity checks: the new edge shape starts where the current edge ends
	assert(newEdgeShape.nearR() == m_r);
	assert(newEdgeShape.phiForR(m_r) == e->m_shape.phiForR(m_r));

	std::shared_ptr<SweepInterval> previousInterval = e->m_previousInterval;
	std::shared_ptr<SweepInterval> nextInterval = e->m_nextInterval;
	m_edges.erase(e->shape());
	SweepEdge newEdge(newEdgeShape);
	auto& insertedEdge = (m_edges.insert(std::make_pair(newEdgeShape, newEdge)))->second;

	insertedEdge.m_previousInterval = previousInterval;
	insertedEdge.m_nextInterval = nextInterval;

	return SwitchResult{previousInterval.get(), &insertedEdge, nextInterval.get()};
}

SweepCircle::MergeResult SweepCircle::mergeToInterval(SweepEdge* rightEdge, SweepEdge* leftEdge) {
	// sanity checks: the edges to merge end at the same phi
	assert(leftEdge->shape().farR() == m_r);
	assert(rightEdge->shape().farR() == m_r);
	assert(leftEdge->shape().phiForR(m_r) == rightEdge->shape().phiForR(m_r));

	std::shared_ptr<SweepInterval> middleInterval = rightEdge->m_nextInterval;
	middleInterval->setPreviousBoundary(rightEdge->m_previousInterval->previousBoundary());
	if (rightEdge->m_previousInterval->previousBoundary()) {
		rightEdge->m_previousInterval->previousBoundary()->m_nextInterval = middleInterval;
	} else {
		m_firstInterval = middleInterval;
	}
	if (leftEdge->m_nextInterval->nextBoundary()) {
		leftEdge->m_nextInterval->nextBoundary()->m_previousInterval = middleInterval;
	} else {
		m_lastInterval = middleInterval;
	}
	middleInterval->setNextBoundary(leftEdge->m_nextInterval->nextBoundary());

	// this erases both edges as they have the same phi value
	m_edges.erase(leftEdge->shape());

	return MergeResult{middleInterval.get()};
}

} // namespace cartocrow::flow_map
