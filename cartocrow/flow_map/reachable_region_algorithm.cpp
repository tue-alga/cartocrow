/*
The Flow Map library implements the algorithmic geo-visualization
method by the same name, developed by Kevin Verbeek, Kevin Buchin,
and Bettina Speckmann at TU Eindhoven
(DOI: 10.1007/s00453-013-9867-z & 10.1109/TVCG.2011.202).
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

#include "reachable_region_algorithm.h"

#include <cmath>
#include <optional>
#include <ostream>

#include "../core/core.h"
#include "intersections.h"
#include "polar_point.h"
#include "polar_segment.h"
#include "spiral_segment.h"
#include "sweep_circle.h"
#include "sweep_interval.h"

namespace cartocrow::flow_map {

ReachableRegionAlgorithm::UnreachableRegionVertex::UnreachableRegionVertex(
    PolarPoint location, std::shared_ptr<SweepEdge> e1, std::shared_ptr<SweepEdge> e2)
    : m_location(location), m_e1(std::move(e1)), m_e2(std::move(e2)) {}

ReachableRegionAlgorithm::Event::Event(PolarPoint position, ReachableRegionAlgorithm* alg)
    : m_position(position), m_alg(alg) {}

Number<Inexact> ReachableRegionAlgorithm::Event::r() const {
	return m_position.r();
}

Number<Inexact> ReachableRegionAlgorithm::Event::phi() const {
	return m_position.phi();
}

bool ReachableRegionAlgorithm::Event::isValid() const {
	return true;
}

void ReachableRegionAlgorithm::Event::insertJoinEvents() {
	if (m_alg->m_circle.isEmpty()) {
		return;
	}
	auto [begin, end] = m_alg->m_circle.edgesAt(phi());
	if (begin == m_alg->m_circle.begin()) {
		insertJoinEventFor(std::prev(m_alg->m_circle.end()));
	} else {
		insertJoinEventFor(std::prev(begin));
	}
	for (auto e = begin; e != end; ++e) {
		insertJoinEventFor(e);
	}
}

void ReachableRegionAlgorithm::Event::insertJoinEventFor(SweepCircle::EdgeMap::iterator rightEdge) {
	SweepInterval* interval = (*rightEdge)->nextInterval();
	if (interval->previousBoundary() == nullptr || interval->nextBoundary() == nullptr) {
		return;
	}
	if (interval->type() == SweepInterval::Type::OBSTACLE) {
		return;
	}
	std::optional<PolarPoint> vanishingPoint = interval->outwardsVanishingPoint(m_alg->m_circle.r());
	if (vanishingPoint) {
		SweepCircle::EdgeMap::iterator nextEdge;
		if (rightEdge == --m_alg->m_circle.end()) {
			nextEdge = m_alg->m_circle.begin();
		} else {
			nextEdge = std::next(rightEdge);
		}
		m_alg->m_queue.push(
		    std::make_shared<JoinEvent>(*vanishingPoint, *rightEdge, *nextEdge, m_alg));
	}
}

ReachableRegionAlgorithm::NodeEvent::NodeEvent(std::shared_ptr<Node> node,
                                               ReachableRegionAlgorithm* alg)
    : Event(node->m_position, alg), m_node(node) {}

void ReachableRegionAlgorithm::NodeEvent::handle() {
	using enum SweepInterval::Type;
	using enum SweepEdgeShape::Type;

	// TODO
	std::cout << "> \033[1mhandling \033[33mnode event\033[0m\n";
	m_alg->m_debugPainting->setMode(renderer::GeometryRenderer::stroke);
	m_alg->m_debugPainting->setStroke(Color{240, 120, 0}, 1);
	m_alg->m_debugPainting->draw(m_alg->m_tree->rootPosition() +
	                             (m_position.toCartesian() - CGAL::ORIGIN));
	m_alg->m_debugPainting->drawText(
	    m_alg->m_tree->rootPosition() + (m_position.toCartesian() - CGAL::ORIGIN), "node");

	SweepInterval* interval = m_alg->m_circle.intervalAt(m_node->m_position.phi());
	if (interval->type() == SweepInterval::Type::REACHABLE) {
		m_alg->m_reachableNodes.push_back(m_node);
	}
}

ReachableRegionAlgorithm::VertexEvent::VertexEvent(PolarPoint position, std::shared_ptr<SweepEdge> e1,
                                                   std::shared_ptr<SweepEdge> e2,
                                                   ReachableRegionAlgorithm* alg)
    : Event(position, alg), m_e1(e1), m_e2(e2), m_side(determineSide()) {}

void ReachableRegionAlgorithm::VertexEvent::handle() {
	using enum SweepInterval::Type;

	std::string side = "";
	switch (m_side) {
	case Side::LEFT:
		side = "left";
		break;
	case Side::RIGHT:
		side = "right";
		break;
	case Side::NEAR:
		side = "near";
		break;
	case Side::FAR:
		side = "far";
	}
	std::cout << "> \033[1mhandling \033[35m" << side << " vertex event\033[0m\n";
	m_alg->m_debugPainting->setMode(renderer::GeometryRenderer::stroke);
	m_alg->m_debugPainting->setStroke(Color{150, 150, 150}, 0.5);
	m_alg->m_debugPainting->draw(m_alg->m_tree->rootPosition() +
	                             (m_position.toCartesian() - CGAL::ORIGIN));
	m_alg->m_debugPainting->drawText(
	    m_alg->m_tree->rootPosition() + (m_position.toCartesian() - CGAL::ORIGIN), side);

	if (m_side == Side::LEFT) {
		handleLeft();
	} else if (m_side == Side::RIGHT) {
		handleRight();
	} else if (m_side == Side::NEAR) {
		handleNear();
	} else if (m_side == Side::FAR) {
		handleFar();
	}

	insertJoinEvents();
}

void ReachableRegionAlgorithm::VertexEvent::handleLeft() {
	using enum SweepInterval::Type;
	using enum SweepEdgeShape::Type;
	SweepInterval* outsideInterval = m_e2->nextInterval();

	if (outsideInterval->type() == SHADOW) { // case 1
		auto result = m_alg->m_circle.switchEdge(m_e2, m_e1);

	} else if (outsideInterval->type() == REACHABLE) { // case 2
		auto spiral = std::make_shared<SweepEdge>(
		    SweepEdgeShape(RIGHT_SPIRAL, m_position, m_alg->m_tree->restrictingAngle()));
		if (spiral->shape().departsOutwardsToLeftOf(m_position.r(), m_e1->shape())) { // case 2a
			auto result = m_alg->m_circle.splitFromEdge(m_e2, m_e1, spiral);
			result.middleInterval->setType(SHADOW);
			m_alg->m_vertices.emplace_back(m_position, spiral, m_e2);
		} else { // case 2b
			auto result = m_alg->m_circle.switchEdge(m_e2, m_e1);
			m_alg->m_vertices.emplace_back(m_position, m_e1, m_e2);
		}

	} else if (outsideInterval->type() == OBSTACLE) {
		// a vertex event cannot have an obstacle interval on the outside
		assert(false);
	}
}

void ReachableRegionAlgorithm::VertexEvent::handleRight() {
	using enum SweepInterval::Type;
	using enum SweepEdgeShape::Type;
	SweepInterval* outsideInterval = m_e1->previousInterval();

	if (outsideInterval->type() == SHADOW) { // case 1
		auto result = m_alg->m_circle.switchEdge(m_e1, m_e2);

	} else if (outsideInterval->type() == REACHABLE) { // case 2
		auto spiral = std::make_shared<SweepEdge>(
		    SweepEdgeShape(LEFT_SPIRAL, m_position, m_alg->m_tree->restrictingAngle()));
		if (m_e2->shape().departsOutwardsToLeftOf(m_position.r(), spiral->shape())) { // case 2a
			auto result = m_alg->m_circle.splitFromEdge(m_e1, spiral, m_e2);
			result.middleInterval->setType(SHADOW);
			m_alg->m_vertices.emplace_back(m_position, m_e1, spiral);
		} else { // case 2b
			auto result = m_alg->m_circle.switchEdge(m_e1, m_e2);
			m_alg->m_vertices.emplace_back(m_position, m_e1, m_e2);
		}

	} else if (outsideInterval->type() == OBSTACLE) {
		// a vertex event cannot have an obstacle interval on the outside
		assert(false);
	}
}

void ReachableRegionAlgorithm::VertexEvent::handleNear() {
	using enum SweepInterval::Type;
	using enum SweepEdgeShape::Type;

	SweepInterval* interval = m_alg->m_circle.intervalAt(phi());
	if (interval->type() == OBSTACLE) {  // case 1
		auto result = m_alg->m_circle.splitFromInterval(m_e1, m_e2);
		result.middleInterval->setType(SHADOW);

	} else if (interval->type() == SHADOW) {  // case 2
		auto result = m_alg->m_circle.splitFromInterval(m_e2, m_e1);
		result.middleInterval->setType(OBSTACLE);

	} else if (interval->type() == REACHABLE) {  // case 3
		auto leftSpiral = std::make_shared<SweepEdge>(
		    SweepEdgeShape(LEFT_SPIRAL, m_position, m_alg->m_tree->restrictingAngle()));
		auto rightSpiral = std::make_shared<SweepEdge>(
		    SweepEdgeShape(RIGHT_SPIRAL, m_position, m_alg->m_tree->restrictingAngle()));

		if (m_e2->shape().departsOutwardsToLeftOf(m_position.r(), leftSpiral->shape())) { // case 3a
			auto result = m_alg->m_circle.splitFromInterval(leftSpiral, m_e2, m_e1);
			result.middleLeftInterval->setType(OBSTACLE);
			result.middleRightInterval->setType(SHADOW);
			m_alg->m_vertices.emplace_back(m_position, m_e1, leftSpiral);

		} else if (rightSpiral->shape().departsOutwardsToLeftOf(m_position.r(),
		                                                        m_e1->shape())) { // case 3b
			auto result = m_alg->m_circle.splitFromInterval(m_e2, m_e1, rightSpiral);
			result.middleLeftInterval->setType(SHADOW);
			result.middleRightInterval->setType(OBSTACLE);
			m_alg->m_vertices.emplace_back(m_position, rightSpiral, m_e2);

		} else { // case 3c
			auto result = m_alg->m_circle.splitFromInterval(m_e2, m_e1);
			result.middleInterval->setType(OBSTACLE);
			m_alg->m_vertices.emplace_back(m_position, m_e1, m_e2);
		}
	}
}

void ReachableRegionAlgorithm::VertexEvent::handleFar() {
	using enum SweepInterval::Type;
	using enum SweepEdgeShape::Type;

	if (m_e1->nextInterval() == m_e2->previousInterval()) { // case 1
		auto previousIntervalType = m_e1->previousInterval()->type();
		auto nextIntervalType = m_e2->nextInterval()->type();

		if (previousIntervalType == nextIntervalType) { // case 1a
			auto result = m_alg->m_circle.mergeToInterval(m_e1, m_e2);
			result.mergedInterval->setType(previousIntervalType);
			if (previousIntervalType == REACHABLE) {
				m_alg->m_vertices.emplace_back(m_position, m_e1, m_e2);
			}
		} else { // case 1b/1c
			SweepEdgeShape::Type spiralType =
			    previousIntervalType == REACHABLE ? LEFT_SPIRAL : RIGHT_SPIRAL;
			auto spiral = std::make_shared<SweepEdge>(
			    SweepEdgeShape(spiralType, m_position, m_alg->m_tree->restrictingAngle()));
			auto result = m_alg->m_circle.mergeToEdge(m_e1, m_e2, spiral);
			if (previousIntervalType == REACHABLE) { // case 1b
				m_alg->m_vertices.emplace_back(m_position, m_e1, spiral);
			} else { // case 1c
				m_alg->m_vertices.emplace_back(m_position, spiral, m_e2);
			}
		}

	} else if (m_e2->nextInterval() == m_e1->previousInterval()) { // case 2
		auto result = m_alg->m_circle.mergeToInterval(m_e2, m_e1);
		result.mergedInterval->setType(OBSTACLE);
		if (m_e2->nextInterval()->type() == REACHABLE) {
			m_alg->m_vertices.emplace_back(m_position, m_e1, m_e2);
		}

	} else {
		assert(false);
	}
}

ReachableRegionAlgorithm::VertexEvent::Side ReachableRegionAlgorithm::VertexEvent::determineSide() {
	if (m_e1->shape().nearR() == r() && m_e2->shape().nearR() == r()) {
		return Side::NEAR;
	} else if (m_e1->shape().farR() == r() && m_e2->shape().farR() == r()) {
		return Side::FAR;
	} else if (m_e1->shape().nearR() == r() && m_e2->shape().farR() == r()) {
		return Side::LEFT;
	} else if (m_e1->shape().farR() == r() && m_e2->shape().nearR() == r()) {
		return Side::RIGHT;
	}
	assert(false); // near or far r (of both m_e1 and m_e2) needs to be equal to r() of event
	return Side::NEAR;
}

ReachableRegionAlgorithm::JoinEvent::JoinEvent(PolarPoint position,
                                               std::weak_ptr<SweepEdge> rightEdge,
                                               std::weak_ptr<SweepEdge> leftEdge,
                                               ReachableRegionAlgorithm* alg)
    : Event(position, alg), m_rightEdge(rightEdge), m_leftEdge(leftEdge) {}

void ReachableRegionAlgorithm::JoinEvent::handle() {
	using enum SweepInterval::Type;
	using enum SweepEdgeShape::Type;

	std::shared_ptr<SweepEdge> rightEdge = m_rightEdge.lock();
	std::shared_ptr<SweepEdge> leftEdge = m_leftEdge.lock();

	std::cout << "> \033[1mhandling \033[34mjoin event\033[0m\n";
	m_alg->m_debugPainting->setMode(renderer::GeometryRenderer::stroke);
	m_alg->m_debugPainting->setStroke(Color{0, 120, 240}, 1);
	m_alg->m_debugPainting->draw(m_alg->m_tree->rootPosition() +
	                             (m_position.toCartesian() - CGAL::ORIGIN));
	m_alg->m_debugPainting->drawText(
	    m_alg->m_tree->rootPosition() + (m_position.toCartesian() - CGAL::ORIGIN), "join");

	SweepInterval* previousInterval = rightEdge->previousInterval();
	SweepInterval* interval = rightEdge->nextInterval();
	SweepInterval* nextInterval = leftEdge->nextInterval();

	if (previousInterval->type() == OBSTACLE && nextInterval->type() == OBSTACLE) {
		// ignore, this is handled by a vertex event

	} else if (previousInterval->type() == REACHABLE && nextInterval->type() == REACHABLE) { // case 1
		auto result = m_alg->m_circle.mergeToInterval(rightEdge, leftEdge);
		rightEdge->shape().pruneFarSide(m_position);
		leftEdge->shape().pruneFarSide(m_position);
		result.mergedInterval->setType(REACHABLE);
		m_alg->m_vertices.emplace_back(m_position, rightEdge, leftEdge);

	} else if (previousInterval->type() == OBSTACLE) { // case 2
		leftEdge->shape().pruneFarSide(m_position);
		if (interval->type() == SHADOW) { // case 2a
			assert(nextInterval->type() == REACHABLE);
			rightEdge->shape().pruneNearSide(m_position);
			auto result = m_alg->m_circle.mergeToEdge(rightEdge, leftEdge, rightEdge);
			m_alg->m_vertices.emplace_back(m_position, rightEdge, leftEdge);
		} else if (interval->type() == REACHABLE) { // case 2b
			assert(nextInterval->type() == SHADOW);
			rightEdge->shape().pruneFarSide(m_position);
			auto result = m_alg->m_circle.mergeToEdge(rightEdge, leftEdge, rightEdge);
			m_alg->m_vertices.emplace_back(m_position, leftEdge, rightEdge);
		} else {
			assert(false);
		}

	} else if (nextInterval->type() == OBSTACLE) { // case 3
		rightEdge->shape().pruneFarSide(m_position);
		if (interval->type() == SHADOW) { // case 3a
			assert(previousInterval->type() == REACHABLE);
			leftEdge->shape().pruneNearSide(m_position);
			auto result = m_alg->m_circle.mergeToEdge(rightEdge, leftEdge, leftEdge);
			m_alg->m_vertices.emplace_back(m_position, rightEdge, leftEdge);
		} else if (interval->type() == REACHABLE) { // case 3b
			assert(previousInterval->type() == SHADOW);
			leftEdge->shape().pruneFarSide(m_position);
			auto result = m_alg->m_circle.mergeToEdge(rightEdge, leftEdge, leftEdge);
			m_alg->m_vertices.emplace_back(m_position, leftEdge, rightEdge);
		} else {
			assert(false);
		}
	}

	insertJoinEvents();
}

bool ReachableRegionAlgorithm::JoinEvent::isValid() const {
	// a join event is invalid if one of its edges has already been deleted
	return !m_rightEdge.expired() && m_rightEdge.lock()->isOnCircle() && !m_leftEdge.expired() &&
	       m_leftEdge.lock()->isOnCircle();
}

ReachableRegionAlgorithm::ReachableRegionAlgorithm(std::shared_ptr<SpiralTree> tree)
    : m_tree(tree), m_debugPainting(std::make_shared<renderer::PaintingRenderer>()),
      m_circle(SweepInterval::Type::REACHABLE) {}

ReachableRegionAlgorithm::ReachableRegion ReachableRegionAlgorithm::run() {

	std::cout << "\033[1m──────────────────────────────────────────────────────────\033[0m\n"
	          << "\033[1m Step 1: Outwards sweep to construct the reachable region \033[0m\n"
	          << "\033[1m──────────────────────────────────────────────────────────\033[0m\n";

	// insert all nodes into the event queue
	for (const std::shared_ptr<Node>& node : m_tree->nodes()) {
		if (node->m_position.r() > 0) {
			m_queue.push(std::make_shared<NodeEvent>(node, this));
		}
	}

	// insert all obstacle vertices into the event queue
	for (SpiralTree::Obstacle& obstacle : m_tree->obstacles()) {
		for (auto e = obstacle.begin(); e != obstacle.end(); e++) {
			std::shared_ptr<SweepEdge> e1 = *e;
			std::shared_ptr<SweepEdge> e2 = ++e == obstacle.end() ? *obstacle.begin() : *e;
			m_queue.push(std::make_shared<VertexEvent>(e2->shape().start(), e1, e2, this));
			e--;
		}
	}

	m_circle.print();
	// main loop, handle all events
	while (!m_queue.empty()) {
		std::shared_ptr<Event> event = m_queue.top();
		m_queue.pop();
		if (!event->isValid()) {
			continue;
		}
		if (m_circle.edges().empty()) {
			m_circle.m_onlyInterval->paintSweepShape(*m_debugPainting, m_circle.r(), event->r());
		} else {
			for (const auto& edge : m_circle.edges()) {
				edge->nextInterval()->paintSweepShape(*m_debugPainting, m_circle.r(), event->r());
			}
		}
		m_circle.grow(event->r());
		m_circle.print();
		event->handle();
		m_circle.print();
		assert(m_circle.isValid());
	}

	return {m_vertices, m_reachableNodes};
}

std::shared_ptr<renderer::GeometryPainting> ReachableRegionAlgorithm::debugPainting() {
	return m_debugPainting;
}

} // namespace cartocrow::flow_map
