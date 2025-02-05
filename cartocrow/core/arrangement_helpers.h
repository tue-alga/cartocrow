#ifndef CARTOCROW_ARRANGEMENT_HELPERS_H
#define CARTOCROW_ARRANGEMENT_HELPERS_H

#include "core.h"
#include <vector>
#include <CGAL/General_polygon_2.h>
#include <CGAL/Arr_landmarks_point_location.h>

namespace cartocrow {
/// Compute bounding box of arrangement.
/// Assumes the arrangement is finite, has at least one vertex, uses line segments as curves, and an exact number type.
template <class Arr>
Rectangle<Exact> bboxExact(const Arr& arr) {
	Number<Exact> xmin = arr.vertices_begin()->point().x();
	Number<Exact> xmax = arr.vertices_begin()->point().x();
	Number<Exact> ymin = arr.vertices_begin()->point().y();
	Number<Exact> ymax = arr.vertices_begin()->point().y();
	for (auto vit = arr.vertices_begin(); vit != arr.vertices_end(); ++vit) {
		if (vit->point().x() > xmax) {
			xmax = vit->point().x();
		}
		if (vit->point().x() < xmin) {
			xmin = vit->point().x();
		}
		if (vit->point().y() > ymax) {
			ymax = vit->point().y();
		}
		if (vit->point().y() < ymin) {
			ymin = vit->point().y();
		}
	}
	return {xmin, ymin, xmax, ymax};
}

/// Compute bounding box of arrangement.
/// Assumes the arrangement is finite, has at least one vertex, uses line segments as curves, and an exact number type.
template <class Arr>
Rectangle<Inexact> bboxInexact(const Arr& arr) {
	Number<Inexact> xmin = CGAL::to_double(arr.vertices_begin()->point().x());
	Number<Inexact> xmax = CGAL::to_double(arr.vertices_begin()->point().x());
	Number<Inexact> ymin = CGAL::to_double(arr.vertices_begin()->point().y());
	Number<Inexact> ymax = CGAL::to_double(arr.vertices_begin()->point().y());
	for (auto vit = arr.vertices_begin(); vit != arr.vertices_end(); ++vit) {
		if (vit->point().x() > xmax) {
			xmax = CGAL::to_double(vit->point().x());
		}
		if (vit->point().x() < xmin) {
			xmin = CGAL::to_double(vit->point().x());
		}
		if (vit->point().y() > ymax) {
			ymax = CGAL::to_double(vit->point().y());
		}
		if (vit->point().y() < ymin) {
			ymin = CGAL::to_double(vit->point().y());
		}
	}
	return {xmin, ymin, xmax, ymax};
}

/// Constructs a general polygon from an arrangement CCB (connected component of the boundary).
/// The returned general polygon has the same orientation as the CCB.
/// The Traits class should be a model of ArrangementDirectionalXMonotoneTraits_2.
template <class Traits, class Ccb>
CGAL::General_polygon_2<Traits> ccb_to_general_polygon(Ccb ccb) {
	Traits traits;
	auto opposite = traits.construct_opposite_2_object();
	auto curr = ccb;

	std::vector<typename Traits::X_monotone_curve_2> x_monotone_curves;
	do {
		auto curve = curr->curve();
		if (curr->source()->point() == curve.source()) {
			x_monotone_curves.push_back(curve);
		} else {
			x_monotone_curves.push_back(opposite(curve));
		}
	} while (++curr != ccb);

	return {x_monotone_curves.begin(), x_monotone_curves.end()};
}

/// Constructs a linear polygon from an arrangement CCB (connected component of the boundary).
/// The returned polygon has the same orientation as the CCB, and its vertices are the vertices of the CCB.
/// The kernel should match the one used in the CCB.
template <class K, class Ccb>
Polygon<K> ccb_to_polygon(Ccb ccb) {
	auto curr = ccb;

	std::vector<Point<K>> points;
	do {
		points.push_back(curr->source()->point());
	} while (++curr != ccb);

	return {points.begin(), points.end()};
}

/// Constructs a linear polygon with holes from a face of an arrangement.
/// The kernel should match the one used in the CCB.
template <class K, class FaceH>
PolygonWithHoles<K> face_to_polygon_with_holes(FaceH fh) {
	Polygon<K> outer;
	if (fh->has_outer_ccb()) {
		outer = ccb_to_polygon<K>(fh->outer_ccb());
	}
	std::vector<Polygon<K>> holes;
	for (auto ccbIt = fh->inner_ccbs_begin(); ccbIt != fh->inner_ccbs_end(); ++ccbIt) {
		holes.push_back(ccb_to_polygon<K>(*ccbIt));
	}

	return {outer, holes.begin(), holes.end()};
}

/// Constructs a general polygon with holes from a face of an arrangement.
/// The Traits class should be a model of ArrangementDirectionalXMonotoneTraits_2.
template <class Traits, class FaceH>
CGAL::General_polygon_with_holes_2<Traits> face_to_general_polygon_with_holes(FaceH fh) {
	CGAL::General_polygon_2<Traits> outer;
	if (fh->has_outer_ccb()) {
		outer = ccb_to_general_polygon<Traits>(fh->outer_ccb());
	}
	std::vector<CGAL::General_polygon_2<Traits>> holes;
	for (auto ccbIt = fh->inner_ccbs_begin(); ccbIt != fh->inner_ccbs_end(); ++ccbIt) {
		holes.push_back(ccb_to_general_polygon<Traits>(*ccbIt));
	}

	return {outer, holes.begin(), holes.end()};
}

/// A collection of faces in an arrangement.
/// These faces need not be connected.
/// The class provides functions for iterating over all vertices, edges, and faces in the component,
/// and ones for determining the boundaries of the component.
/// The interface mimics that of a Face in an arrangement.
template <class Arr>
class Component {
  public:
	//	typedef std::vector<FaceH>::iterator Face_iterator;
	typedef Arr::Face_handle FaceH;
	typedef Arr::Size Size;
	typedef Arr::Halfedge_handle HalfedgeH;
	typedef Arr::Vertex_handle VertexH;
	typedef Arr::Vertex_const_handle VertexCH;
	typedef Arr::Traits_2 Traits;

	class ComponentCcbCirculator {
	  private:
		using Self = ComponentCcbCirculator;
		Arr::Halfedge_handle m_halfedge;
		std::function<bool(FaceH)> m_in_component;

	  public:
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = Arr::Halfedge;
		using difference_type = std::ptrdiff_t;
		using pointer = Arr::Halfedge_handle;
		using reference = value_type&;

		ComponentCcbCirculator(Arr::Halfedge_handle halfedge, std::function<bool(FaceH)> in_component)
		    : m_halfedge(halfedge), m_in_component(std::move(in_component)) {};

		value_type operator*() const {
			return *m_halfedge;
		}

		pointer operator->() const {
			return m_halfedge;
		}

		pointer ptr() const {
			return m_halfedge;
		}

		pointer handle() const {
			return m_halfedge;
		}

		Self& operator++() {
			m_halfedge = m_halfedge->next();
			while (m_in_component(m_halfedge->twin()->face())) {
				m_halfedge = m_halfedge->twin()->next();
			}
			return *this;
		};

		Self operator++(int) {
			Self tmp = *this;
			this->operator++();
			return tmp;
		}

		Self& operator--() {
			m_halfedge = m_halfedge->prev();
			while (m_in_component(m_halfedge->twin()->face())) {
				m_halfedge = m_halfedge->twin()->prev();
			}
			return *this;
		};

		Self operator--(int) {
			Self tmp = *this;
			this->operator--();
			return tmp;
		}

		bool operator==(const Self& other) const {
			return m_halfedge == other.m_halfedge;
		}

		bool operator!=(const Self& other) const {
			return m_halfedge != other.m_halfedge;
		}
	};

	class Face_const_iterator {
	  private:
		using Self = Face_const_iterator;
		std::vector<FaceH>::const_iterator m_faceHandleIterator;

	  public:
		using iterator_category = std::input_iterator_tag;
		using value_type = Arr::Face;
		using difference_type = std::ptrdiff_t;
		using pointer = Arr::Face_handle;
		using reference = value_type&;

		Face_const_iterator(std::vector<FaceH>::const_iterator faceHandleIterator) :
		      m_faceHandleIterator(faceHandleIterator) {};

		value_type operator*() const {
			return *(*m_faceHandleIterator);
		}

		pointer operator->() const {
			return *m_faceHandleIterator;
		}

		Self& operator++() {
			++m_faceHandleIterator;
			return *this;
		};

		Self operator++(int) {
			Self tmp = *this;
			this->operator++();
			return tmp;
		}

		pointer ptr() const {
			return *m_faceHandleIterator;
		}

		pointer handle() const {
			return *m_faceHandleIterator;
		}

		bool operator==(const Self& other) const {
			return m_faceHandleIterator == other.m_faceHandleIterator;
		}

		bool operator!=(const Self& other) const {
			return m_faceHandleIterator != other.m_faceHandleIterator;
		}
	};

	class Edge_iterator {
	  private:
		using Self = Edge_iterator;
		Face_const_iterator m_faces_it;

		// Ccbs of current face.
		std::vector<typename Arr::Ccb_halfedge_circulator> m_ccbs;
		// Current ccb
		std::vector<typename Arr::Ccb_halfedge_circulator>::const_iterator m_ccbIt;
		// Current halfedge
		HalfedgeH m_halfedge;

		std::function<bool(FaceH)> m_in_component;

		bool m_startOfFace = true;

		bool m_skipTwins;

	  public:
		using iterator_category = std::input_iterator_tag;
		using value_type = Arr::Halfedge;
		using difference_type = std::ptrdiff_t;
		using pointer = HalfedgeH;
		using reference = value_type&;

		Edge_iterator(Face_const_iterator faces_it, std::function<bool(FaceH)> in_component, bool skipTwins) :
		      m_faces_it(std::move(faces_it)), m_in_component(std::move(in_component)), m_skipTwins(skipTwins) {};

		pointer operator->() const {
			// Invariant m_halfedge is one we want to "output" or startOfFace holds
			if (m_startOfFace) {
				auto fh = m_faces_it.handle();
				typename Arr::Ccb_halfedge_circulator ccb;
				if (fh->has_outer_ccb()) {
					ccb = fh->outer_ccb();
				} else {
					ccb = *fh->inner_ccbs_begin();
				}
				while (m_skipTwins && m_in_component(ccb->face()) && m_in_component(ccb->twin()->face()) &&
				       ccb->direction() == CGAL::ARR_RIGHT_TO_LEFT) {
					++ccb;
				}
				return ccb;
			}
			return m_halfedge;
		}

		value_type operator*() const {
			return *(operator->());
		}

		Self& operator++() {
			if (m_startOfFace) {
				// Update the ccbs
				m_ccbs.clear();
				auto fh = m_faces_it.handle();
				if (fh->has_outer_ccb()) {
					m_ccbs.push_back(fh->outer_ccb());
				}
				std::copy(fh->inner_ccbs_begin(), fh->inner_ccbs_end(), std::back_inserter(m_ccbs));

				// Initialize rest
				m_ccbIt = m_ccbs.begin();
				m_halfedge = m_ccbIt->ptr();

				while (m_skipTwins && m_in_component(m_halfedge->face()) && m_in_component(m_halfedge->twin()->face()) &&
				       m_halfedge->direction() == CGAL::ARR_RIGHT_TO_LEFT) {
					m_halfedge = m_halfedge->next();
				}
			}
			do {
				m_halfedge = m_halfedge->next();
			} while (m_skipTwins && m_halfedge != m_ccbIt->ptr() && m_in_component(m_halfedge->face()) &&
			         m_in_component(m_halfedge->twin()->face()) && m_halfedge->direction() == CGAL::ARR_RIGHT_TO_LEFT);
			// Skip inner edges that are directed from right to left.

			m_startOfFace = false;

			// Done with this ccb
			if (m_halfedge == m_ccbIt->ptr()) {
				++m_ccbIt;
				// Done with all ccbs of this face
				if (m_ccbIt == m_ccbs.end()) {
					++m_faces_it;
					m_startOfFace = true;
				} else {
					// Not yet done with the ccbs
					// Update halfedge
					m_halfedge = m_ccbIt->ptr();
					while (m_skipTwins && m_in_component(m_halfedge->face()) && m_in_component(m_halfedge->twin()->face()) &&
					       m_halfedge->direction() == CGAL::ARR_RIGHT_TO_LEFT) {
						m_halfedge = m_halfedge->next();
					}
				}
			}

			return *this;
		};

		Self operator++(int) {
			Self tmp = *this;
			this->operator++();
			return tmp;
		}

		pointer ptr() const {
			return operator->();
		}

		pointer handle() const {
			return operator->();
		}

		bool operator==(const Self& other) const {
			return m_faces_it == other.m_faces_it && (m_startOfFace && other.m_startOfFace || m_halfedge == other.m_halfedge);
		}

		bool operator!=(const Self& other) const {
			return m_faces_it != other.m_faces_it || ((!m_startOfFace || !other.m_startOfFace) && m_halfedge != other.m_halfedge);
		}
	};

	class Vertex_iterator {
	  private:
		using Self = Vertex_iterator;
		Face_const_iterator m_faces_it;
		typename Arr::Isolated_vertex_iterator m_vit;
		bool m_startOfFace = true;
		Size m_n;

	  public:
		using iterator_category = std::input_iterator_tag;
		using value_type = Arr::Vertex;
		using difference_type = std::ptrdiff_t;
		using pointer = VertexH;
		using reference = value_type&;

		Vertex_iterator(Face_const_iterator faces_it, int n) : m_faces_it(std::move(faces_it)), m_n(n) {
			if (m_n > 0) {
				while (m_faces_it->number_of_isolated_vertices() == 0) {
					++m_faces_it;
				}
				m_vit = m_faces_it->isolated_vertices_begin();
			}
		}

		pointer operator->() const {
			return m_vit.ptr();
		}

		value_type operator*() const {
			return *m_vit;
		}

		Self& operator++() {
			--m_n;
			if (m_n == 0) return *this;
			if (m_vit == m_faces_it->isolated_vertices_end()) {
				while (m_faces_it->number_of_isolated_vertices() == 0) {
					++m_faces_it;
				}
				m_vit = m_faces_it->isolated_vertices_begin();
			} else {
				++m_vit;
			}

			return *this;
		}

		Self operator++(int) {
			Self tmp = *this;
			this->operator++();
			return tmp;
		}

		pointer ptr() const {
			return operator->();
		}

		pointer handle() const {
			return operator->();
		}

		bool operator==(const Self& other) const {
			return m_n == other.m_n;
		}

		bool operator!=(const Self& other) const {
			return m_n != other.m_n;
		}
	};

    /// Construct a component from its constituent faces, its boundary, and a predicate function
    /// that can (efficiently) determine whether a face is part of the component or not.
	Component(std::vector<FaceH> faces, std::vector<HalfedgeH> boundary_edges, std::function<bool(FaceH)> in_component) :
	      m_faces(std::move(faces)), m_in_component(std::move(in_component)) {
		for (auto fh : m_faces) {
			m_nr_isolated_vertices += fh->number_of_isolated_vertices();
		}

		typename Arr::Traits_2 traits;
		auto opposite = traits.construct_opposite_2_object();
		while (!boundary_edges.empty()) {
			auto he = boundary_edges.front();
			auto circ_start = ComponentCcbCirculator(he, m_in_component);
			auto circ = circ_start;

			std::vector<typename Arr::X_monotone_curve_2> xm_curves;
			auto last_it = boundary_edges.end();
			do {
				last_it = std::remove(boundary_edges.begin(), last_it, circ.handle());
				auto curve = circ->curve();
				if (circ->source()->point() == curve.source()) {
					xm_curves.push_back(curve);
				} else {
					xm_curves.push_back(opposite(curve));
				}
			} while (++circ != circ_start);
			boundary_edges.erase(last_it, boundary_edges.end());

			CGAL::General_polygon_2<typename Arr::Traits_2> polygon(xm_curves.begin(), xm_curves.end());
			auto orientation = polygon.orientation();
			if (orientation == CGAL::COUNTERCLOCKWISE) {
				m_outer_ccbs.push_back(circ_start);
			} else if (orientation == CGAL::CLOCKWISE) {
				m_inner_ccbs.push_back(circ_start);
			} else {
				throw std::runtime_error("Face orientation is not clockwise nor counterclockwise.");
			}
		}
	}

	bool has_outer_ccb() const {
		return !m_outer_ccbs.empty();
	}
	Size number_of_outer_ccbs() const {
		return m_outer_ccbs.size();
	}
	std::vector<ComponentCcbCirculator>::const_iterator outer_ccbs_begin() const {
		return m_outer_ccbs.cbegin();
	}
	std::vector<ComponentCcbCirculator>::const_iterator outer_ccbs_end() const {
		return m_outer_ccbs.cend();
	}

	ComponentCcbCirculator outer_ccb() const {
		return m_outer_ccbs[0];
	}
	std::vector<ComponentCcbCirculator>::const_iterator inner_ccbs_begin() const {
		return m_inner_ccbs.cbegin();
	}
	std::vector<ComponentCcbCirculator>::const_iterator inner_ccbs_end() const {
		return m_inner_ccbs.cend();
	}
	Size number_of_inner_ccbs() const {
		return m_inner_ccbs.size();
	}

	// Hole is an alias for inner_ccb
	std::vector<ComponentCcbCirculator>::const_iterator holes_begin() const {
		return inner_ccbs_begin();
	}
	std::vector<ComponentCcbCirculator>::const_iterator holes_end() const {
		return inner_ccbs_end();
	}
	Size number_of_holes() const {
		return number_of_inner_ccbs();
	}

	Face_const_iterator faces_begin() const {
		return {m_faces.cbegin()};
	}
	Face_const_iterator faces_end() const {
		return {m_faces.cend()};
	}

    /// Return the surface of the component as a polygon with holes.
    /// The vertices of this polygon are the vertices of the component.
	PolygonWithHoles<Exact> surface_polygon() const {
		Polygon<Exact> outer;
		if (has_outer_ccb()) {
			outer = ccb_to_polygon<Exact>(outer_ccb());
		}
		std::vector<Polygon<Exact>> holes;
		for (auto ccbIt = inner_ccbs_begin(); ccbIt != inner_ccbs_end(); ++ccbIt) {
			holes.push_back(ccb_to_polygon<Exact>(*ccbIt));
		}

		return {outer, holes.begin(), holes.end()};
	}

    /// Return the surface of the component as a general polygon with holes.
    /// The Traits class should be a model of ArrangementDirectionalXMonotoneTraits_2.
	CGAL::General_polygon_with_holes_2<Traits> surface() const {
		CGAL::General_polygon_2<Traits> outer;
		if (has_outer_ccb()) {
			outer = ccb_to_general_polygon<Traits>(outer_ccb());
		}
		std::vector<CGAL::General_polygon_2<Traits>> holes;
		for (auto ccbIt = inner_ccbs_begin(); ccbIt != inner_ccbs_end(); ++ccbIt) {
			holes.push_back(ccb_to_general_polygon<Traits>(*ccbIt));
		}

		return {outer, holes.begin(), holes.end()};
	}

	Edge_iterator edges_begin() const {
		return Edge_iterator(faces_begin(), m_in_component, true);
	}

	Edge_iterator edges_end() const {
		return Edge_iterator(faces_end(), m_in_component, true);
	}

	Edge_iterator halfedges_begin() const {
		return Edge_iterator(faces_begin(), m_in_component, false);
	}

	Edge_iterator halfedges_end() const {
		return Edge_iterator(faces_end(), m_in_component, false);
	}

	Size number_of_isolated_vertices() const {
		return m_nr_isolated_vertices;
	}

	Vertex_iterator isolated_vertices_begin() const {
		return Vertex_iterator(faces_begin(), m_nr_isolated_vertices);
	}

	Vertex_iterator isolated_vertices_end() const {
		return Vertex_iterator(faces_end(), 0);
	}

  public:
	/// Return an arrangement that consists only of this component.
	/// This function creates a new arrangement with all halfedges of this component.
	/// Note that vertex, edge, and face data are not copied.
	/// Also note that currently the function does not copy over isolated vertices within the component.
	/// The function copyBoundedFaceData can be used to copy over face data of bounded faces.
	Arr arrangement() const {
		Arr arr;

		std::vector<typename Arr::X_monotone_curve_2> xm_curves;
		for (auto eit = edges_begin(); eit != edges_end(); ++eit) {
			xm_curves.push_back(eit->curve());
		}
		CGAL::insert_non_intersecting_curves(arr, xm_curves.begin(), xm_curves.end());

		// todo: fix below
//		for (auto vit = isolated_vertices_begin(); vit != isolated_vertices_end(); ++vit) {
//			typename Traits::Point_2 p = vit->point();
//			CGAL::insert(arr, p);
//		}
		return arr;

	}

  private:
	std::vector<FaceH> m_faces;
	std::function<bool(FaceH)> m_in_component;
	std::vector<ComponentCcbCirculator> m_outer_ccbs;
	std::vector<ComponentCcbCirculator> m_inner_ccbs;
	int m_nr_isolated_vertices;
};

/// Compute connected components of faces that satisfy the predicate.
/// The components are output as Component objects.
/// \sa Component
template <class Arr, class OutputIterator>
void connectedComponents(const Arr& arr, OutputIterator out, const std::function<bool(typename Arr::Face_handle)>& in_component) {
	typedef typename Arr::Face_handle FaceH;
	typedef typename Arr::Halfedge_handle HalfEdgeH;
	std::vector<FaceH> remaining;
	for (auto fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) {
		auto fh = fit.ptr();
		if (in_component(fh)) {
			remaining.emplace_back(fh);
		}
	}

	while (!remaining.empty()) {
		// We do a BFS
		std::vector<FaceH> compFaces;
		std::vector<HalfEdgeH> compBoundaryEdges;
		auto first = remaining.front();
		std::deque<FaceH> q;
		q.push_back(first);

		while (!q.empty()) {
			auto f = q.front();
			q.pop_front();
			compFaces.push_back(f);

			// Go through boundaries of this face
			std::vector<typename Arr::Ccb_halfedge_circulator> ccbs;
			std::copy(f->outer_ccbs_begin(), f->outer_ccbs_end(), std::back_inserter(ccbs));
			std::copy(f->inner_ccbs_begin(), f->inner_ccbs_end(), std::back_inserter(ccbs));
			for (auto ccb_start : ccbs) {
				auto ccb_it = ccb_start;

				// Go through each neighbouring face
				do {
					auto candidate = ccb_it->twin()->face();
					if (!in_component(candidate)) {
						compBoundaryEdges.emplace_back(ccb_it.ptr());
					} else {
						// If this is one of the provided faces, and not yet added to queue or compFaces, add it to queue.
						if (std::find(compFaces.begin(), compFaces.end(), candidate) ==
						        compFaces.end() &&
						    std::find(q.begin(), q.end(), candidate) == q.end()) {
							q.push_back(candidate);
						}
					}
				} while (++ccb_it != ccb_start);
			}
		}

		// Done with this connected component
		remaining.erase(std::remove_if(remaining.begin(), remaining.end(), [&compFaces](const auto& f) {
			                return std::find(compFaces.begin(), compFaces.end(), f) != compFaces.end();
		                }), remaining.end());
		*out++ = Component<Arr>(std::move(compFaces), std::move(compBoundaryEdges), in_component);
	}
}

/// Copy face data of bounded faces from arr1 to arr2.
/// Precondition: arr2 is a subset of arr1.
template <class ArrWithFaceData>
void copyBoundedFaceData(const ArrWithFaceData& arr1, ArrWithFaceData& arr2) {
	using Arr = ArrWithFaceData;
	using FaceH = Arr::Face_handle;
	using HalfedgeH = Arr::Halfedge_handle;
	using VertexH = Arr::Vertex_handle;
	using VertexCH = Arr::Vertex_const_handle;
	using CCB = typename Arr::Ccb_halfedge_circulator;

	std::vector<FaceH> remainingBoundedFaces;
	for (auto fit = arr2.faces_begin(); fit != arr2.faces_end(); ++fit) {
		auto fh = fit.ptr();
		if (!fh->is_unbounded()) {
			remainingBoundedFaces.emplace_back(fh);
		}
	}

	using Landmarks_pl = CGAL::Arr_landmarks_point_location<Arr>;
	Landmarks_pl pl(arr1);

	while (!remainingBoundedFaces.empty()) {
		// A bounded face of arr2
		auto fh2 = remainingBoundedFaces.front();

		// We will locate face fh of arr2 in arr1.
		auto ccb2 = fh2->has_outer_ccb() ? fh2->outer_ccb() : *fh2->inner_ccbs_begin();
		auto vt2 = ccb2->target();
		auto vt2Pt = vt2->point();
		auto vs2 = ccb2->source();
		auto vs2Pt = vs2->point();

		auto obj = pl.locate(vt2Pt);
		HalfedgeH ccb1;
		if (auto vt1_p = boost::get<VertexCH>(&obj)) {
			VertexCH vt1 = *vt1_p;
			// Convention: the target of these halfedges is vt_.
			auto circ = vt1->incident_halfedges();
			auto curr = circ;
			bool found = false;
			do {
				if (curr->source()->point() == vs2Pt) {
					ccb1 = curr.ptr();
					found = true;
				}
			} while (++curr != circ);
			if (!found) {
				// This should never occur.
				throw std::runtime_error("Could not find halfedge in copy of arrangement.");
			}
		} else {
			throw std::runtime_error(
			    "Copy of arrangement does not contain same vertices (is the arrangement exact?)");
		}

		// This is the face fh2 in arr1.
		FaceH fh1 = ccb2->face() == fh2 ? ccb1->face() : ccb1->twin()->face();
		assert(!fh1->is_unbounded());
		std::unordered_map<FaceH, double> map;

		std::deque<std::pair<FaceH, FaceH>> q;
		q.push_back({fh2, fh1});

		std::vector<FaceH> visited2;

		while (!q.empty()) {
			auto [f2, f1] = q.front();
			q.pop_front();
			f2->set_data(f1->data());

			visited2.push_back(f2);

			// Go through boundaries of this face
			std::vector<std::pair<CCB, CCB>> ccbs;
			ccbs.push_back({f2->outer_ccb(), f1->outer_ccb()});
			std::vector<CCB> iCcbs1;
			std::vector<CCB> iCcbs2;
			std::copy(f2->inner_ccbs_begin(), f2->inner_ccbs_end(), std::back_inserter(iCcbs2));
			std::copy(f1->inner_ccbs_begin(), f1->inner_ccbs_end(), std::back_inserter(iCcbs1));

			std::sort(iCcbs1.begin(), iCcbs1.end(), [](const CCB& ccb1, const CCB& ccb2) {
				return ccb1->source()->point() < ccb2->source()->point();
			});
			std::sort(iCcbs2.begin(), iCcbs2.end(), [](const CCB& ccb1, const CCB& ccb2) {
			  return ccb1->source()->point() < ccb2->source()->point();
			});
			auto f2iccbIt = iCcbs2.begin();
			auto f1iccbIt = iCcbs1.begin();
			auto f2iccbItEnd = iCcbs2.end();
			auto f1iccbItEnd = iCcbs1.end();
			while (f2iccbIt != f2iccbItEnd) {
				ccbs.push_back({*f2iccbIt, *f1iccbIt});
				++f2iccbIt;
				++f1iccbIt;
			}

			for (auto [ccb2_start, ccb1_start] : ccbs) {
				auto ccb1_it = ccb1_start;
				auto ccb2_it = ccb2_start;
				while (ccb1_it->source()->point() != ccb2_it->source()->point()) {
					++ccb1_it;
				}

				// Go through each neighbouring face
				do {
					auto candidate = ccb2_it->twin()->face();
					auto candidate1 = ccb1_it->twin()->face();
					if (!candidate->is_unbounded()) {
						// If this is one of the bounded faces, and not yet added to queue or visited, add it to queue.
						if (std::find(visited2.begin(), visited2.end(), candidate) == visited2.end() &&
						    std::find(q.begin(), q.end(), std::pair(candidate, candidate1)) == q.end()) {
							q.push_back({candidate, candidate1});
						}
					}
					++ccb1_it;
					++ccb2_it;
				} while (ccb2_it != ccb2_start);
			}
		}
		// Done with this connected component
		remainingBoundedFaces.erase(
		    std::remove_if(remainingBoundedFaces.begin(), remainingBoundedFaces.end(),
		                   [&visited2](const auto& f) {
			                   return std::find(visited2.begin(), visited2.end(), f) !=
			                          visited2.end();
		                   }),
		    remainingBoundedFaces.end());
	}
}
}

#endif //CARTOCROW_ARRANGEMENT_HELPERS_H
