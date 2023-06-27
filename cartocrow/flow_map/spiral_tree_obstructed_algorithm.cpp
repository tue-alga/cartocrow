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

#include "spiral_tree_obstructed_algorithm.h"

#include <cmath>
#include <memory>
#include <optional>
#include <ostream>

#include "../core/core.h"
#include "cartocrow/flow_map/reachable_region_algorithm.h"
#include "intersections.h"
#include "polar_point.h"
#include "polar_segment.h"
#include "spiral_segment.h"
#include "sweep_circle.h"
#include "sweep_interval.h"

namespace cartocrow::flow_map {

SpiralTreeObstructedAlgorithm::Event::Event(PolarPoint position,
                                            SpiralTreeObstructedAlgorithm* alg)
    : m_position(position), m_alg(alg) {}

Number<Inexact> SpiralTreeObstructedAlgorithm::Event::r() const {
	return m_position.r();
}

Number<Inexact> SpiralTreeObstructedAlgorithm::Event::phi() const {
	return m_position.phi();
}

bool SpiralTreeObstructedAlgorithm::Event::isValid() const {
	return true;
}

void SpiralTreeObstructedAlgorithm::Event::insertJoinEvents() {
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

void SpiralTreeObstructedAlgorithm::Event::insertJoinEventFor(
    SweepCircle::EdgeMap::iterator rightEdge) {
	SweepInterval* interval = (*rightEdge)->nextInterval();
	if (interval->previousBoundary() == nullptr || interval->nextBoundary() == nullptr) {
		return;
	}
	if (interval->type() == SweepInterval::Type::OBSTACLE) {
		return;
	}
	std::optional<PolarPoint> vanishingPoint = interval->inwardsVanishingPoint(m_alg->m_circle.r());
	if (vanishingPoint) {
		SweepCircle::EdgeMap::iterator nextEdge;
		if (rightEdge == --m_alg->m_circle.end()) {
			nextEdge = m_alg->m_circle.begin();
		} else {
			nextEdge = std::next(rightEdge);
		}
		m_alg->m_queue.push(
		    std::make_shared<JoinEvent>(*vanishingPoint, *rightEdge, *nextEdge, m_alg));
		/*std::cout << "(inserted join event at r = " << vanishingPoint->r() << " for ["
		          << interval->previousBoundary()->shape().nearEndpoint().toCartesian();
		if (interval->previousBoundary()->shape().farEndpoint()) {
			std::cout << " – " << interval->previousBoundary()->shape().farEndpoint()->toCartesian();
		}
		std::cout << "]  →  [" << interval->nextBoundary()->shape().nearEndpoint().toCartesian();
		if (interval->nextBoundary()->shape().farEndpoint()) {
			std::cout << " – " << interval->nextBoundary()->shape().farEndpoint()->toCartesian();
		}
		std::cout << "])" << std::endl;*/
	} else {
		/*std::cout << "(ignored join event for ["
		          << interval->previousBoundary()->shape().nearEndpoint().toCartesian();
		if (interval->previousBoundary()->shape().farEndpoint()) {
			std::cout << " – " << interval->previousBoundary()->shape().farEndpoint()->toCartesian();
		}
		std::cout << "]  →  [" << interval->nextBoundary()->shape().nearEndpoint().toCartesian();
		if (interval->nextBoundary()->shape().farEndpoint()) {
			std::cout << " – " << interval->nextBoundary()->shape().farEndpoint()->toCartesian();
		}
		std::cout << "])" << std::endl;*/
	}
}

SpiralTreeObstructedAlgorithm::NodeEvent::NodeEvent(std::shared_ptr<Node> node,
                                                    SpiralTreeObstructedAlgorithm* alg)
    : Event(node->m_position, alg), m_node(node) {}

void SpiralTreeObstructedAlgorithm::NodeEvent::handle() {
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

	// TODO do this only if the node is reachable from the origin
	auto leftSpiral = std::make_shared<SweepEdge>(
	    SweepEdgeShape(LEFT_SPIRAL, m_position, m_alg->m_tree->restrictingAngle()));
	auto rightSpiral = std::make_shared<SweepEdge>(
	    SweepEdgeShape(RIGHT_SPIRAL, m_position, m_alg->m_tree->restrictingAngle()));
	auto result = m_alg->m_circle.splitFromInterval(leftSpiral, rightSpiral);
	result.middleInterval->setType(REACHABLE);
	result.middleInterval->setNode(m_node);

	insertJoinEvents();
}

SpiralTreeObstructedAlgorithm::VertexEvent::VertexEvent(PolarPoint position,
                                                        std::shared_ptr<SweepEdge> e1,
                                                        std::shared_ptr<SweepEdge> e2,
                                                        SpiralTreeObstructedAlgorithm* alg)
    : Event(position, alg), m_e1(e1), m_e2(e2), m_side(determineSide()) {}

void SpiralTreeObstructedAlgorithm::VertexEvent::handle() {
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

void SpiralTreeObstructedAlgorithm::VertexEvent::handleLeft() {
	using enum SweepInterval::Type;
	using enum SweepEdgeShape::Type;
	SweepInterval* outsideInterval = m_e1->nextInterval();
	if (outsideInterval->type() == FREE) {
		auto result = m_alg->m_circle.switchEdge(m_e1, m_e2);
	} else if (outsideInterval->type() == REACHABLE) {
		auto spiral = std::make_shared<SweepEdge>(
		    SweepEdgeShape(LEFT_SPIRAL, m_position, m_alg->m_tree->restrictingAngle()));
		if (spiral->shape().departsInwardsToLeftOf(m_position.r(), m_e2->shape())) {
			auto result = m_alg->m_circle.splitFromEdge(m_e1, m_e2, spiral);
			result.middleInterval->setType(FREE);
		} else {
			auto result = m_alg->m_circle.switchEdge(m_e1, m_e2);
		}
	} else if (outsideInterval->type() == OBSTACLE) {
		// a vertex event cannot have an obstacle interval on the outside
		assert(false);
	}
}

void SpiralTreeObstructedAlgorithm::VertexEvent::handleRight() {
	using enum SweepInterval::Type;
	using enum SweepEdgeShape::Type;
	SweepInterval* outsideInterval = m_e2->previousInterval();
	if (outsideInterval->type() == FREE) {
		auto result = m_alg->m_circle.switchEdge(m_e2, m_e1);
	} else if (outsideInterval->type() == REACHABLE) {
		auto spiral = std::make_shared<SweepEdge>(
		    SweepEdgeShape(LEFT_SPIRAL, m_position, m_alg->m_tree->restrictingAngle()));
		if (m_e1->shape().departsInwardsToLeftOf(m_position.r(), spiral->shape())) {
			auto result = m_alg->m_circle.splitFromEdge(m_e2, spiral, m_e1);
			result.middleInterval->setType(FREE);
		} else {
			auto result = m_alg->m_circle.switchEdge(m_e2, m_e1);
		}
	} else if (outsideInterval->type() == OBSTACLE) {
		// a vertex event cannot have an obstacle interval on the outside
		assert(false);
	}
}

void SpiralTreeObstructedAlgorithm::VertexEvent::handleNear() {
	using enum SweepInterval::Type;
	using enum SweepEdgeShape::Type;

	// concave corners of obstacles shouldn't be possible
	assert(m_e2->nextInterval() == m_e1->previousInterval());
	auto previousIntervalType = m_e2->previousInterval()->type();
	auto nextIntervalType = m_e1->nextInterval()->type();

	if (previousIntervalType == nextIntervalType) {
		auto result = m_alg->m_circle.mergeToInterval(m_e2, m_e1);
		result.mergedInterval->setType(previousIntervalType);
		// TODO: if previousIntervalType == REACHABLE then join nodes
	} else {
		SweepEdgeShape::Type spiralType =
			previousIntervalType == REACHABLE ? RIGHT_SPIRAL : LEFT_SPIRAL;
		auto spiral = std::make_shared<SweepEdge>(
			SweepEdgeShape(spiralType, m_position, m_alg->m_tree->restrictingAngle()));
		auto result = m_alg->m_circle.mergeToEdge(m_e2, m_e1, spiral);
		// TODO: figure out what Kevin's implementation does here
		// probably set some parent pointers so we can route the spiral edge?
	}
}

void SpiralTreeObstructedAlgorithm::VertexEvent::handleFar() {
	using enum SweepInterval::Type;
	using enum SweepEdgeShape::Type;

	SweepInterval* interval = m_alg->m_circle.intervalAt(phi());
	if (interval->type() == FREE) {
		auto result = m_alg->m_circle.splitFromInterval(m_e1, m_e2);
		result.middleInterval->setType(OBSTACLE);
	} else if (interval->type() == OBSTACLE) {
		auto result = m_alg->m_circle.splitFromInterval(m_e2, m_e1);
		result.middleInterval->setType(FREE);
	}
}

SpiralTreeObstructedAlgorithm::VertexEvent::Side
SpiralTreeObstructedAlgorithm::VertexEvent::determineSide() {
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

SpiralTreeObstructedAlgorithm::JoinEvent::JoinEvent(PolarPoint position,
                                                    std::weak_ptr<SweepEdge> rightEdge,
                                                    std::weak_ptr<SweepEdge> leftEdge,
                                                    SpiralTreeObstructedAlgorithm* alg)
    : Event(position, alg), m_rightEdge(rightEdge), m_leftEdge(leftEdge) {}

void SpiralTreeObstructedAlgorithm::JoinEvent::handle() {
	using enum SweepInterval::Type;
	using enum SweepEdgeShape::Type;

	std::shared_ptr<SweepEdge> rightEdge = m_rightEdge.lock();
	std::shared_ptr<SweepEdge> leftEdge = m_leftEdge.lock();

	std::cout << "> \033[1mhandling \033[34mjoin event\033[0m";
	m_alg->m_debugPainting->setMode(renderer::GeometryRenderer::stroke);
	m_alg->m_debugPainting->setStroke(Color{0, 120, 240}, 1);
	m_alg->m_debugPainting->draw(m_alg->m_tree->rootPosition() +
	                             (m_position.toCartesian() - CGAL::ORIGIN));
	m_alg->m_debugPainting->drawText(
	    m_alg->m_tree->rootPosition() + (m_position.toCartesian() - CGAL::ORIGIN), "join");

	SweepInterval* previousInterval = rightEdge->previousInterval();
	SweepInterval* interval = rightEdge->nextInterval();
	SweepInterval* nextInterval = leftEdge->nextInterval();

	std::shared_ptr<Node> rightNode = previousInterval->node();
	std::shared_ptr<Node> leftNode = nextInterval->node();

	if (previousInterval->type() == OBSTACLE && nextInterval->type() == OBSTACLE) {
		// ignore, this is handled by a vertex event
		std::cout << " (ignored)\n";

	} else if (previousInterval->type() == FREE && nextInterval->type() == FREE) {
		// case 1: simply merge the intervals into a big reachable interval
		std::cout << " (case 1)\n";

		auto result = m_alg->m_circle.mergeToInterval(rightEdge, leftEdge);
		rightEdge->shape().pruneNearSide(m_position);
		leftEdge->shape().pruneNearSide(m_position);
		result.mergedInterval->setType(REACHABLE);

	} else if (previousInterval->type() == REACHABLE && nextInterval->type() == REACHABLE) {
		// case 1.5: join and make new active node
		std::cout << " (case 1.5)\n";

		rightEdge->shape().pruneNearSide(m_position);
		leftEdge->shape().pruneNearSide(m_position);
		auto rightSpiral = std::make_shared<SweepEdge>(
			SweepEdgeShape(RIGHT_SPIRAL, m_position, m_alg->m_tree->restrictingAngle()));
		auto leftSpiral = std::make_shared<SweepEdge>(
			SweepEdgeShape(LEFT_SPIRAL, m_position, m_alg->m_tree->restrictingAngle()));
		//auto result = m_alg->m_circle.mergeAndSplit(rightEdge, leftEdge, leftSpiral, rightSpiral);
		m_alg->m_circle.mergeToInterval(rightEdge, leftEdge);
		auto result = m_alg->m_circle.splitFromInterval(leftSpiral, rightSpiral);
		result.rightInterval->setType(FREE);
		result.middleInterval->setType(REACHABLE);
		result.leftInterval->setType(FREE);
		m_alg->m_circle.mergeFreeIntervals();

		std::shared_ptr<Node> node = std::make_shared<Node>(m_position);
		m_alg->m_tree->m_nodes.push_back(node);
		result.middleInterval->setNode(node);

		if (rightNode != nullptr) {
			m_alg->m_tree->addEdge(node, rightNode);
		}
		if (leftNode != nullptr) {
			m_alg->m_tree->addEdge(node, leftNode);
		}

	} else if ((previousInterval->type() == REACHABLE && nextInterval->type() == FREE) ||
	           (previousInterval->type() == FREE && nextInterval->type() == REACHABLE)) {
		// this case is in Kevin's code (line 1177-1193) but I have no idea why
		// this can ever happen. assert(false) for now so if this ever fails
		// then I know how it can happen :) addendum: it can actually happen
		// because two adjacent free intervals aren't merged after a previous
		// join, Kevin's code didn't merge such intervals
		assert(false); // TODO

	} else if (previousInterval->type() == OBSTACLE) {
		// case 2: right side is obstacle
		std::cout << " (case 2)\n";

		leftEdge->shape().pruneNearSide(m_position);
		if (interval->type() == FREE) {
			// case 2a: shadow in the middle, reachable on the left
			assert(nextInterval->type() == REACHABLE);
			auto node = nextInterval->node();
			rightEdge->shape().pruneNearSide(m_position);
			auto result = m_alg->m_circle.mergeToEdge(rightEdge, leftEdge, rightEdge);
			result.leftInterval->setNode(node);
		} else if (interval->type() == REACHABLE) {
			// case 2b: reachable in the middle, shadow on the left
			assert(nextInterval->type() == FREE);
			rightEdge->shape().pruneNearSide(m_position);
			auto result = m_alg->m_circle.mergeToEdge(rightEdge, leftEdge, rightEdge);
		} else {
			assert(false);
		}

	} else if (nextInterval->type() == OBSTACLE) {
		// case 3: left side is obstacle
		std::cout << " (case 3)\n";

		rightEdge->shape().pruneNearSide(m_position);
		if (interval->type() == FREE) {
			assert(previousInterval->type() == REACHABLE);
			auto node = previousInterval->node();
			leftEdge->shape().pruneNearSide(m_position);
			auto result = m_alg->m_circle.mergeToEdge(rightEdge, leftEdge, leftEdge);
			result.rightInterval->setNode(node);
		}
	}

	insertJoinEvents();
}

bool SpiralTreeObstructedAlgorithm::JoinEvent::isValid() const {
	// a join event is invalid if one of its edges has already been deleted
	return !m_rightEdge.expired() && !m_leftEdge.expired();
}

SpiralTreeObstructedAlgorithm::SpiralTreeObstructedAlgorithm(
    std::shared_ptr<SpiralTree> tree,
    std::vector<ReachableRegionAlgorithm::UnreachableRegionVertex> vertices)
    : m_tree(tree), m_vertices(std::move(vertices)),
      m_debugPainting(std::make_shared<renderer::PaintingRenderer>()),
      m_circle(SweepInterval::Type::FREE) {}

void SpiralTreeObstructedAlgorithm::run() {

	std::cout << "\033[1m────────────────────────────────────────────────────\033[0m\n"
	          << "\033[1m Step 2: Inwards sweep to construct the spiral tree \033[0m\n"
	          << "\033[1m────────────────────────────────────────────────────\033[0m\n";

	// insert all terminals into the event queue
	for (const std::shared_ptr<Node>& node : m_tree->nodes()) {
		if (node->m_position.r() > 0) {
			m_queue.push(std::make_shared<NodeEvent>(node, this));
		}
	}

	// insert vertices of the unreachable region
	for (ReachableRegionAlgorithm::UnreachableRegionVertex& vertex : m_vertices) {
		// TODO debug drawing
		/*assert(vertex.m_e1->shape().farEndpoint().has_value());
		assert(vertex.m_e2->shape().farEndpoint().has_value());
		m_debugPainting->setStroke(Color{120, 230, 120}, 2);
		for (Number<Inexact> r = vertex.m_e1->shape().nearR(); r < *vertex.m_e1->shape().farR();
		     r *= 1.01) {
			m_debugPainting->draw(
			    Segment<Inexact>(vertex.m_e1->shape().evalForR(r).toCartesian(),
			                     vertex.m_e1->shape()
			                         .evalForR(std::min(*vertex.m_e1->shape().farR(), r * 1.01))
			                         .toCartesian()));
		}
		m_debugPainting->draw(
		    (vertex.m_e1->shape()
		         .evalForR((vertex.m_location.r() + *vertex.m_e1->shape().averageR()) / 2)
		         .toCartesian()));
		m_debugPainting->setStroke(Color{230, 120, 120}, 2);
		for (Number<Inexact> r = vertex.m_e2->shape().nearR(); r < *vertex.m_e2->shape().farR();
		     r *= 1.01) {
			m_debugPainting->draw(
			    Segment<Inexact>(vertex.m_e2->shape().evalForR(r).toCartesian(),
			                     vertex.m_e2->shape()
			                         .evalForR(std::min(*vertex.m_e2->shape().farR(), r * 1.01))
			                         .toCartesian()));
		}
		m_debugPainting->draw(
		    (vertex.m_e2->shape()
		         .evalForR((vertex.m_location.r() + *vertex.m_e2->shape().averageR()) / 2)
		         .toCartesian()));
		m_debugPainting->setStroke(Color{220, 20, 70}, 1);
		m_debugPainting->draw(vertex.m_location.toCartesian());*/
		m_queue.push(std::make_shared<VertexEvent>(vertex.m_location, vertex.m_e1, vertex.m_e2, this));
	}

	if (m_queue.size() == 0) {
		// TODO error or something
		return;
	}

	m_circle.grow(m_queue.top()->r());
	int eventCount = 0; // TODO debug: limit number of events handled
	// main loop, handle all events
	while (!m_queue.empty() /*&& eventCount++ < 100*/) {
		std::shared_ptr<Event> event = m_queue.top();
		m_queue.pop();
		if (!event->isValid()) {
			continue;
		}
		if (m_circle.edges().empty()) {
			m_circle.m_onlyInterval->paintSweepShape(*m_debugPainting, event->r(), m_circle.r());
		} else {
			for (const auto& edge : m_circle.edges()) {
				edge->nextInterval()->paintSweepShape(*m_debugPainting, event->r(), m_circle.r());
			}
		}
		m_circle.shrink(event->r());
		m_circle.print();
		event->handle();
		m_circle.print();
		assert(m_circle.isValid());

		// TODO temporary
		if (!m_circle.isValid()) {
			break;
		}
	}
}

std::shared_ptr<renderer::GeometryPainting> SpiralTreeObstructedAlgorithm::debugPainting() {
	return m_debugPainting;
}

} // namespace cartocrow::flow_map
