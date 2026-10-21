#include "../catch.hpp"

#include <cartocrow/core/core.h>
#include <cartocrow/data_structures/straight_graph_2.h>

using namespace cartocrow;

TEST_CASE("Graph static vertex map") {

	using G = Straight_graph_2<std::monostate, std::monostate, Exact, PlainGraph<false>>;
	using V = G::Vertex_handle;
	using P = Point<Exact>;

	G g;
	g.initialize();
	V u = g.add_vertex(P(0, 0));
	V v = g.add_vertex(P(0, 0));

	Graph_static_vertex_map<G,int> map(g, 1);

	CHECK(map[u] == 1);
	CHECK(map[v] == 1);

	int a = map[u];
		
	for (V v : g.vertices()) {
		map[v] = 7;
	}

	map[u] = 3;
	map[v] = 2;

	int b = map[u];

	CHECK(map[u] == 3);
	CHECK(map[v] == 2);

	CHECK(a == 1);
	CHECK(b == 3);
}

TEST_CASE("Graph vertex map") {

	using G = Straight_graph_2<std::monostate, std::monostate, Exact, PlainGraph<false>>;
	using V = G::Vertex_handle;
	using P = Point<Exact>;

	G g;
	g.initialize();
	V u = g.add_vertex(P(0, 0));
	V v = g.add_vertex(P(0, 0));

	Graph_vertex_map<G, int> map(g, 1);

	CHECK(map[u] == 1);
	CHECK(map[v] == 1);

	map[u] = 3;
	map[v] = 2;

	CHECK(map[u] == 3);
	CHECK(map[v] == 2);

	g.remove_vertex(u);

	CHECK(map[v] == 2);

	map[v] = 5;

	CHECK(map[v] == 5);
}

TEST_CASE("Graph static edge map") {

	using G = Straight_graph_2<std::monostate, std::monostate, Exact, PlainGraph<false>>;
	using V = G::Vertex_handle;
	using E = G::Edge_handle;
	using P = Point<Exact>;

	G g;
	g.initialize();
	V u = g.add_vertex(P(0, 0));
	V v = g.add_vertex(P(0, 0));
	V w = g.add_vertex(P(0, 0));


	E e = g.add_edge(u, v);
	E f = g.add_edge(v, w);

	std::cout << *u << std::endl;
	std::cout << *e << std::endl;

	Graph_static_edge_map<G, int> map(g, 1);

	CHECK(map[e] == 1);
	CHECK(map[f] == 1);

	map[e] = 3;
	map[f] = 2;

	CHECK(map[e] == 3);
	CHECK(map[f] == 2);
}

TEST_CASE("Graph edge map") {

	using G = Straight_graph_2<std::monostate, std::monostate, Exact, PlainGraph<false>>;
	using V = G::Vertex_handle;
	using E = G::Edge_handle;
	using P = Point<Exact>;

	G g;
	g.initialize();
	V u = g.add_vertex(P(0, 0));
	V v = g.add_vertex(P(0, 0));
	V w = g.add_vertex(P(0, 0));

	E e = g.add_edge(u, v);
	E f = g.add_edge(v, w);

	Graph_edge_map<G, int> map(g, 1);

	CHECK(map[e] == 1);
	CHECK(map[f] == 1);

	map[e] = 3;
	map[f] = 2;

	CHECK(map[e] == 3);
	CHECK(map[f] == 2);

	g.remove_edge(e);

	CHECK(map[f] == 2);

	map[f] = 5;

	CHECK(map[f] == 5);
}

TEST_CASE("Graph history") {

	using G = Straight_graph_2<std::monostate, std::monostate, Exact, PlainGraph<true>>;
	using V = G::Vertex_handle;
	using E = G::Edge_handle;
	using P = Point<Exact>;

	G g;
	g.initialize();
	V u = g.add_vertex(P(0, 0));
	V v = g.add_vertex(P(0, 0));
	V w = g.add_vertex(P(0, 0));

	auto& h = g.history();

	CHECK(g.number_of_vertices() == 3);
	CHECK(h.can_undo());
	CHECK(!h.can_redo());

	h.undo();

	CHECK(g.number_of_vertices() == 2);
	CHECK(h.can_undo());
	CHECK(h.can_redo());

	h.undo();

	CHECK(g.number_of_vertices() == 1);
	CHECK(h.can_undo());
	CHECK(h.can_redo());

	h.undo();

	CHECK(g.number_of_vertices() == 0);
	CHECK(!h.can_undo());
	CHECK(h.can_redo());
	
	h.redo();

	CHECK(g.number_of_vertices() == 1);
	CHECK(h.can_undo());
	CHECK(h.can_redo());

	h.redo();
	
	CHECK(g.number_of_vertices() == 2);
	CHECK(h.can_undo());
	CHECK(h.can_redo());

	h.redo();

	CHECK(g.number_of_vertices() == 3);
	CHECK(h.can_undo());
	CHECK(!h.can_redo());
}

TEST_CASE("Graph operations") {

	using G = Straight_graph_2<std::monostate, std::monostate, Exact, DecomposedGraph<true, std::monostate>>;
	using V = G::Vertex_handle;
	using E = G::Edge_handle;
	using Path = G::Path_handle;
	using P = Point<Exact>;

	G g;
	V u = g.add_vertex(P(0, 0));
	V v = g.add_vertex(P(1, 1));
	V w = g.add_vertex(P(2, 0));

	E e = g.add_edge(u, v);
	E f = g.add_edge(v, w);

	g.initialize();

	CHECK(g.is_initialized());
	CHECK(g.number_of_vertices() == 3);
	CHECK(g.number_of_edges() == 2);

	E ne = g.split_vertex(v, P(0,1), P(2,1));

	CHECK(g.is_initialized());
	CHECK(g.number_of_vertices() == 4);
	CHECK(g.number_of_edges() == 3);

	V nv = g.collapse_edge(ne, P(0, 10));

	CHECK(g.is_initialized());
	CHECK(g.number_of_vertices() == 3);
	CHECK(g.number_of_edges() == 2);

	V nv2 = g.subdivide_edge(nv->outgoing(), P(10, 10));

	CHECK(g.is_initialized());
	CHECK(g.number_of_vertices() == 4);
	CHECK(g.number_of_edges() == 3);

	g.merge_vertex(nv2);

	CHECK(g.is_initialized());
	CHECK(g.number_of_vertices() == 3);
	CHECK(g.number_of_edges() == 2);
}

TEST_CASE("Graph copy") {

	using G1 = Straight_graph_2<std::monostate, std::monostate, Exact, OrientedGraph<false>>;
	using V1 = G1::Vertex_handle;
	using E1 = G1::Edge_handle;
	using G2 = Straight_graph_2<std::monostate, std::monostate, Exact, DecomposedGraph<true, std::monostate>>;
	using P = Point<Exact>;

	G1 g;
	V1 u = g.add_vertex(P(0, 0));
	V1 v = g.add_vertex(P(1, 1));
	V1 w = g.add_vertex(P(2, 0));

	E1 e = g.add_edge(u, v);
	E1 f = g.add_edge(v, w);

	g.initialize();

	CHECK(g.is_initialized());
	CHECK(g.number_of_vertices() == 3);
	CHECK(g.number_of_edges() == 2);

	G2 g2;
	graph_2_copy(g, g2);

	CHECK(g2.is_initialized());
	CHECK(g2.number_of_vertices() == 3);
	CHECK(g2.number_of_edges() == 2);

	using G3 = Straight_graph_2<std::monostate, std::monostate, Inexact, DecomposedGraph<true, std::monostate>>;

	G3 g3;
	graph_2_copy(g, g3);
	graph_2_copy(g2, g3);
}