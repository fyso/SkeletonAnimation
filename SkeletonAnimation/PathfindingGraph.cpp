#include "PathfindingGraph.h"

Animation::CPathfindingGraph::CPathfindingGraph() : CBaseGraph(0)
{
	m_AdjacencyMatrix = std::make_shared<AdjacencyMatrix>(0);
}

Animation::CPathfindingGraph::CPathfindingGraph(const CPathfindingGraph & vOther) : CBaseGraph(vOther)
{
	m_AdjacencyMatrix = vOther.m_AdjacencyMatrix;
	m_NodeCount = vOther.m_NodeCount;
}

Animation::CPathfindingGraph::CPathfindingGraph(int vNodeCount) : CBaseGraph(vNodeCount)
{
	m_AdjacencyMatrix = std::make_shared<AdjacencyMatrix>(vNodeCount);
}

void Animation::CPathfindingGraph::addEdgeV(int vStartNodeIndex, int vEndNodeIndex)
{
	_ASSERT(vStartNodeIndex >= 0 && vStartNodeIndex < m_NodeCount);
	_ASSERT(vEndNodeIndex >= 0 && vEndNodeIndex < m_NodeCount);

	if (!hasEdgeV(vStartNodeIndex, vEndNodeIndex))
	{
		boost::add_edge(vStartNodeIndex, vEndNodeIndex, 1, *m_AdjacencyMatrix);
	}
}

void Animation::CPathfindingGraph::deleteEdgeV(int vStartNodeIndex, int vEndNodeIndex)
{
	_ASSERT(vStartNodeIndex >= 0 && vStartNodeIndex < m_NodeCount);
	_ASSERT(vEndNodeIndex >= 0 && vEndNodeIndex < m_NodeCount);

	if (hasEdgeV(vStartNodeIndex, vEndNodeIndex))
	{
		boost::remove_edge(vStartNodeIndex, vEndNodeIndex, *m_AdjacencyMatrix);
	}
}

std::set<int> Animation::CPathfindingGraph::getNeighborOfNodeV(int vIndex) const
{
	_ASSERT(vIndex >= 0 && vIndex < m_NodeCount);

	std::set<int> Neighbors;
	IndexMap Indices = boost::get(vertex_index, *m_AdjacencyMatrix);

	AdjacencyVertexIterator ai;
	AdjacencyVertexIterator ai_end;
	for (tie(ai, ai_end) = adjacent_vertices(vIndex, *m_AdjacencyMatrix); ai != ai_end; ++ai)
	{
		Neighbors.insert(static_cast<int>(Indices[*ai]));
	}
	return Neighbors;
}

int Animation::CPathfindingGraph::findShortestDistance(int vSourceNodeIndex, int vDestinationNodeIndex)
{
	std::vector<int> Path(m_NodeCount);
	std::vector<int> Distance(m_NodeCount);
	/*
	VertexDescriptor Src = vertex(vSourceNodeIndex, *m_AdjacencyMatrix);
	VertexDescriptor Targ = vertex(vDestinationNodeIndex, *m_AdjacencyMatrix);
	*/

	boost::dijkstra_shortest_paths(
		*m_AdjacencyMatrix, 
		vSourceNodeIndex,
		boost::predecessor_map(&Path[0]).distance_map(&Distance[0])
	);

	int Result = Distance[vDestinationNodeIndex];
	if (Result == std::numeric_limits<int>().max())
		return -1;
	else 
		return Result;
}

bool Animation::CPathfindingGraph::hasEdgeV(int vStartNodeIndex, int vEndNodeIndex)
{
	_ASSERT(vStartNodeIndex >= 0 && vStartNodeIndex < m_NodeCount);
	_ASSERT(vEndNodeIndex >= 0 && vEndNodeIndex < m_NodeCount);

	std::set<int> Neigbors = getNeighborOfNodeV(vStartNodeIndex);
	return Neigbors.find(vEndNodeIndex) != Neigbors.end();
}

void Animation::CPathfindingGraph::clearAndReinitializeGraphDataV(int vNodeCount)
{
	m_NodeCount = vNodeCount;
	m_AdjacencyMatrix = std::make_shared<AdjacencyMatrix>(vNodeCount);
}
