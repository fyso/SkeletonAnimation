#pragma once
#include "BaseGraph.h"
#include "Common.h"
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/graph_utility.hpp>

#include <boost/graph/astar_search.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

using namespace boost;
typedef adjacency_matrix<undirectedS,boost::no_property, property<edge_weight_t, int>> AdjacencyMatrix;
typedef property_map<AdjacencyMatrix, vertex_index_t>::type IndexMap;
typedef graph_traits<AdjacencyMatrix>::adjacency_iterator AdjacencyVertexIterator;
typedef graph_traits<AdjacencyMatrix>::vertex_descriptor VertexDescriptor;

namespace Animation
{
#define CPathfindingGraphPtr std::shared_ptr<pbd::CPathfindingGraph>
#define CPathfindingGraphMaker std::make_shared<pbd::CPathfindingGraph>
	class CPathfindingGraph : public CBaseGraph
	{
	public:
		CPathfindingGraph();
		CPathfindingGraph(int vNodeCount);
		CPathfindingGraph(const CPathfindingGraph& vOther);

		/*虚方法*/
		virtual void addEdgeV(int vStartNodeIndex, int vEndNodeIndex) override;
		virtual void deleteEdgeV(int vStartNodeIndex, int vEndNodeIndex) override;
		virtual bool hasEdgeV(int vStartNodeIndex, int vEndNodeIndex) override;
		virtual void clearAndReinitializeGraphDataV(int vNodeCount) override;
		virtual std::set<int> getNeighborOfNodeV(int vIndex) const override;

		/*功能性方法*/
		int findShortestDistance(int vSourceNodeIndex, int vDestinationNodeIndex);

	private:
		std::shared_ptr<AdjacencyMatrix> m_AdjacencyMatrix;
	};
}

