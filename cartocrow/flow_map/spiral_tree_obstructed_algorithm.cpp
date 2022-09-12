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
#include <optional>
#include <ostream>

#include <glog/logging.h>

#include "../core/core.h"
#include "../necklace_map/circular_range.h"
#include "circulator.h"
#include "intersections.h"
#include "polar_segment.h"
#include "spiral_segment.h"
#include "sweep_circle.h"
#include "sweep_interval.h"

namespace cartocrow::flow_map {

SpiralTreeObstructedAlgorithm::Event::Event(PolarPoint position, Type type)
    : m_position(position), m_type(type) {}

Number<Inexact> SpiralTreeObstructedAlgorithm::Event::r() const {
	return m_position.r();
}

Number<Inexact> SpiralTreeObstructedAlgorithm::Event::phi() const {
	return m_position.phi();
}

SpiralTreeObstructedAlgorithm::Event::Type SpiralTreeObstructedAlgorithm::Event::type() const {
	return m_type;
}

SpiralTreeObstructedAlgorithm::NodeEvent::NodeEvent(PolarPoint position)
    : Event(position, Type::NODE) {}

void SpiralTreeObstructedAlgorithm::NodeEvent::handle(SpiralTreeObstructedAlgorithm& alg) {
	// TODO
	std::cout << "> handling node event" << std::endl;
	alg.m_debugPainting->setMode(renderer::GeometryRenderer::stroke);
	alg.m_debugPainting->setStroke(Color{240, 120, 0}, 1);
	alg.m_debugPainting->draw(alg.m_tree->rootPosition() + (m_position.toCartesian() - CGAL::ORIGIN));
	alg.m_debugPainting->drawText(
	    alg.m_tree->rootPosition() + (m_position.toCartesian() - CGAL::ORIGIN), "node");
}

SpiralTreeObstructedAlgorithm::VertexEvent::VertexEvent(PolarPoint position,
                                                        std::shared_ptr<SweepEdge> e1,
                                                        std::shared_ptr<SweepEdge> e2)
    : Event(position, Type::VERTEX), m_e1(e1), m_e2(e2), m_side(determineSide()) {}

void SpiralTreeObstructedAlgorithm::VertexEvent::handle(SpiralTreeObstructedAlgorithm& alg) {
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
	std::cout << "> handling " << side << " vertex event" << std::endl;
	alg.m_debugPainting->setMode(renderer::GeometryRenderer::stroke);
	alg.m_debugPainting->setStroke(Color{150, 150, 150}, 0.5);
	alg.m_debugPainting->draw(alg.m_tree->rootPosition() + (m_position.toCartesian() - CGAL::ORIGIN));
	alg.m_debugPainting->drawText(
	    alg.m_tree->rootPosition() + (m_position.toCartesian() - CGAL::ORIGIN), side);

	if (m_side == Side::NEAR) {
		SweepInterval* interval = alg.m_circle.intervalAt(phi());
		if (interval->type() == SweepInterval::Type::REACHABLE) {
			SweepCircle::SplitResult result = alg.m_circle.splitFromInterval(m_e2, m_e1);
			result.middleInterval->setType(SweepInterval::Type::OBSTACLE);
		} else if (interval->type() == SweepInterval::Type::OBSTACLE) {
			SweepCircle::SplitResult result = alg.m_circle.splitFromInterval(m_e1, m_e2);
			result.middleInterval->setType(SweepInterval::Type::REACHABLE); // ???
		} else if (interval->type() == SweepInterval::Type::SHADOW) {
			// ???
		}

	} else if (m_side == Side::FAR) {
		SweepInterval* interval = m_e1->nextInterval();
		SweepCircle::MergeResult result = alg.m_circle.mergeToInterval(m_e1, m_e2);
		if (interval->type() == SweepInterval::Type::REACHABLE) {
			result.mergedInterval->setType(SweepInterval::Type::OBSTACLE);
		} else if (interval->type() == SweepInterval::Type::OBSTACLE) {
			result.mergedInterval->setType(SweepInterval::Type::REACHABLE);
		} else if (interval->type() == SweepInterval::Type::SHADOW) {
			// ???
		}

	} else if (m_side == Side::LEFT) {
		SweepInterval* i = m_e2->nextInterval();
		if (i->type() == SweepInterval::Type::SHADOW) {
			SweepCircle::SwitchResult result = alg.m_circle.switchEdge(m_e2, m_e1);
			// TODO
		} else if (i->type() == SweepInterval::Type::REACHABLE) {
			auto spiral = std::make_shared<SweepEdge>(SweepEdgeShape(
			    SweepEdgeShape::Type::RIGHT_SPIRAL, m_position, alg.m_tree->restrictingAngle()));
			if (spiral->shape().departsToLeftOf(m_e1->shape())) {
				SweepCircle::SplitResult result = alg.m_circle.splitFromEdge(m_e2, m_e1, spiral);
				result.middleInterval->setType(SweepInterval::Type::SHADOW);
			} else {
				SweepCircle::SwitchResult result = alg.m_circle.switchEdge(m_e2, m_e1);
			}
			// TODO
		} else if (i->type() == SweepInterval::Type::OBSTACLE) {
			// a vertex event cannot have an obstacle interval on the outside
			assert(false);
		}

	} else if (m_side == Side::RIGHT) {
		SweepInterval* i = m_e1->previousInterval();
		SweepCircle::SwitchResult result = alg.m_circle.switchEdge(m_e1, m_e2);
		if (i->type() == SweepInterval::Type::SHADOW) {
			// ...
		} else if (i->type() == SweepInterval::Type::REACHABLE) {
			// ...
		} else if (i->type() == SweepInterval::Type::OBSTACLE) {
			// a vertex event cannot have an obstacle interval on the outside
			assert(false);
		}
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
}

SpiralTreeObstructedAlgorithm::JoinEvent::JoinEvent(PolarPoint position)
    : Event(position, Type::JOIN) {}

void SpiralTreeObstructedAlgorithm::JoinEvent::handle(SpiralTreeObstructedAlgorithm& alg) {
	// TODO
	std::cout << "> handling join event" << std::endl;
	alg.m_debugPainting->setMode(renderer::GeometryRenderer::stroke);
	alg.m_debugPainting->setStroke(Color{0, 120, 240}, 1);
	alg.m_debugPainting->draw(alg.m_tree->rootPosition() + (m_position.toCartesian() - CGAL::ORIGIN));
	alg.m_debugPainting->drawText(
	    alg.m_tree->rootPosition() + (m_position.toCartesian() - CGAL::ORIGIN), "join");
}

SpiralTreeObstructedAlgorithm::SpiralTreeObstructedAlgorithm(std::shared_ptr<SpiralTree> tree)
    : m_tree(tree), m_debugPainting(std::make_shared<renderer::PaintingRenderer>()) {}

void SpiralTreeObstructedAlgorithm::run() {

	// insert all terminals into the event queue
	EventQueue events;
	for (const std::shared_ptr<Node>& node : m_tree->nodes()) {
		if (node->m_position.r() > 0) {
			events.push(std::make_shared<NodeEvent>(node->m_position));
		}
	}
	for (SpiralTree::Obstacle& obstacle : m_tree->obstacles()) {
		for (auto e = obstacle.begin(); e != obstacle.end(); e++) {
			std::shared_ptr<SweepEdge> e1 = *e;
			std::shared_ptr<SweepEdge> e2 = ++e == obstacle.end() ? *obstacle.begin() : *e;
			events.push(std::make_shared<VertexEvent>(e2->shape().start(), e1, e2));
			e--;
		}
	}

	m_circle.print();
	int eventCount = 0; // TODO debug: limit number of events handled
	// main loop, handle all events
	while (!events.empty() && eventCount++ < 500) {
		std::shared_ptr<Event> event = events.top();
		events.pop();
		m_circle.firstInterval().paintSweepShape(*m_debugPainting, m_circle.r(), event->r());
		for (const auto& edge : m_circle.edges()) {
			edge.second->nextInterval()->paintSweepShape(*m_debugPainting, m_circle.r(), event->r());
		}
		m_circle.grow(event->r());
		event->handle(*this);
		m_circle.print();
	}
}

std::shared_ptr<renderer::GeometryPainting> SpiralTreeObstructedAlgorithm::debugPainting() {
	return m_debugPainting;
}

} // namespace cartocrow::flow_map
