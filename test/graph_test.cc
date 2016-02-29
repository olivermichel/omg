#include <catch.h>

#include <omg/graph.h>

TEST_CASE("graph vertices and edges", "[Graph]")
{
	omg::Graph<int, int> g1;

	REQUIRE(typeid(g1) == typeid(omg::Graph<int,int>));

	SECTION("graphs are empty upon construction")
	{
		REQUIRE(g1.vertices.count() == 0);
		REQUIRE(g1.edges.count() == 0);
	}

	SECTION("vertices can be added")
	{
		g1.vertices.add(1);
		g1.vertices.add(2);
		REQUIRE(g1.vertices.count() == 2);
	}

	SECTION("vertices can be connected through edges")
	{
		g1.vertices.add(1);
		g1.vertices.add(2);
		g1.edges.add(g1.vertices[1], g1.vertices[2], 1);
		REQUIRE(g1.edges.count() == 1);
	}

	SECTION("vertices and edges can be accessed through sequential indices")
	{
		g1.vertices.add(1);
		g1.vertices.add(2);
		g1.vertices.add(3);

		REQUIRE_NOTHROW(g1.vertices[3]);
		REQUIRE_THROWS(g1.vertices[4]);
		REQUIRE_THROWS(g1.vertices[0]);

		g1.edges.add(g1.vertices[1], g1.vertices[2], 1);
		g1.edges.add(g1.vertices[2], g1.vertices[3], 2);

		REQUIRE_NOTHROW(g1.edges[1]);
		REQUIRE_NOTHROW(g1.edges[2]);
		REQUIRE_THROWS(g1.edges[0]);
		REQUIRE_THROWS(g1.edges[3]);
	}
}

TEST_CASE("sub and super graphs", "[Graph]")
{
	omg::Graph<int, int> g1;
	omg::Graph<int, int> g2;

	SECTION("map graph objects onto each other")
	{
		REQUIRE(typeid(g1) == typeid(omg::Graph<int,int>));
		REQUIRE(typeid(g2) == typeid(omg::Graph<int,int>));

		REQUIRE(!g1.has_subgraphs());
		REQUIRE(!g2.supergraph());

		g1.map(&g2);

		REQUIRE(g1.has_subgraphs());
		REQUIRE(g1.has_subgraph(&g2));
		REQUIRE(g2.supergraph());
	}

	SECTION("map and unmap graph vertices")
	{
		auto a = g1.vertices.add(1);
		auto b = g2.vertices.add(1);

		REQUIRE(!g1.vertices[1].has_subvertices());
		REQUIRE(!g1.vertices[1].has_supervertex());
		REQUIRE(!g2.vertices[1].has_subvertices());
		REQUIRE(!g2.vertices[1].has_supervertex());

		REQUIRE(g1.vertices[1].sub_vertices().empty());
		REQUIRE(g1.vertices[1].super_vertex() == nullptr);
		REQUIRE(g2.vertices[1].sub_vertices().empty());
		REQUIRE(g2.vertices[1].super_vertex() == nullptr);

		g1.vertices[1].map(g2.vertices[1]);

		REQUIRE(g1.vertices[1].has_subvertices());
		REQUIRE(!g1.vertices[1].has_supervertex());
		REQUIRE(g2.vertices[1].has_supervertex());
		REQUIRE(!g2.vertices[1].has_subvertices());

		REQUIRE(g1.vertices[1].sub_vertices()[0] == b._ptr());
		REQUIRE(g1.vertices[1].super_vertex() == nullptr);
		REQUIRE(g2.vertices[1].sub_vertices().empty());
		REQUIRE(g2.vertices[1].super_vertex() == a._ptr());

		g1.vertices[1].unmap(g2.vertices[1]);

		REQUIRE(!g1.vertices[1].has_subvertices());
		REQUIRE(!g1.vertices[1].has_supervertex());
		REQUIRE(!g2.vertices[1].has_subvertices());
		REQUIRE(!g2.vertices[1].has_supervertex());

		REQUIRE(g1.vertices[1].sub_vertices().empty());
		REQUIRE(g1.vertices[1].super_vertex() == nullptr);
		REQUIRE(g2.vertices[1].sub_vertices().empty());
		REQUIRE(g2.vertices[1].super_vertex() == nullptr);
	}

	SECTION("map and unmap graph edges")
	{
		g1.vertices.add(1);
		g1.vertices.add(2);
		g2.vertices.add(1);
		g2.vertices.add(2);

		auto a = g1.edges.add(g1.vertices[1], g1.vertices[2], 1);
		auto b = g2.edges.add(g2.vertices[1], g2.vertices[2], 1);

		REQUIRE(!g1.edges[1].has_subedges());
		REQUIRE(!g1.edges[1].has_superedge());
		REQUIRE(!g2.edges[1].has_subedges());
		REQUIRE(!g2.edges[1].has_superedge());

		REQUIRE(g1.edges[1].sub_edges().empty());
		REQUIRE(g1.edges[1].super_edge().empty());
		REQUIRE(g2.edges[1].sub_edges().empty());
		REQUIRE(g2.edges[1].super_edge().empty());

		g1.edges[1].map_link(g2.edges[1]);

		REQUIRE(g1.edges[1].has_subedges());
		REQUIRE(!g1.edges[1].has_superedge());
		REQUIRE(g2.edges[1].has_superedge());
		REQUIRE(!g2.edges[1].has_subedges());

		REQUIRE(g1.edges[1].sub_edges()[0] == b._ptr());
		REQUIRE(g1.edges[1].super_edge().empty());
		REQUIRE(g2.edges[1].sub_edges().empty());
		REQUIRE(g2.edges[1].super_edge()[0] == a._ptr());

		g1.edges[1].unmap(g2.edges[1]);

		REQUIRE(!g1.edges[1].has_subedges());
		REQUIRE(!g1.edges[1].has_superedge());
		REQUIRE(!g2.edges[1].has_subedges());
		REQUIRE(!g2.edges[1].has_superedge());

		REQUIRE(g1.edges[1].sub_edges().empty());
		REQUIRE(g1.edges[1].super_edge().empty());
		REQUIRE(g2.edges[1].sub_edges().empty());
		REQUIRE(g2.edges[1].super_edge().empty());
	}
}
