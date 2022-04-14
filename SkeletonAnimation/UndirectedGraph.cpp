#include "UndirectedGraph.h"
#include <boost/dynamic_bitset.hpp>
#include <fstream>
#include <queue>
#include <algorithm>
#include <iostream>
#include <stack>
using namespace Animation;

struct Node
{
	int Index = -1;
	int Distance = INT_MAX;

	Node(int vIndex, int vDistance) : Index(vIndex), Distance(vDistance) {}

	bool operator <(const Node& rhs) const
	{
		//Note: The shorter the distance, the higher the priority
		return Distance > rhs.Distance;
	}

	bool operator <=(const Node& rhs) const
	{
		return Distance >= rhs.Distance;
	}
};

CUndirectedGraph::CUndirectedGraph(int vNodeCount) : m_NodeCount(vNodeCount)
{
	_ASSERTE(m_NodeCount > 0);

	m_AdjacentMatrix = Eigen::MatrixXi(m_NodeCount, m_NodeCount);
	m_AdjacentTable.resize(m_NodeCount);
	m_NodeDegree.resize(m_NodeCount);

#pragma omp parallel for
	for (int i = 0; i < m_NodeCount; i++)
	{
		for (int k = 0; k < m_NodeCount; k++)
		{
			m_AdjacentMatrix(i, k) = (i == k ? 0 : MAX_DISTANCE);
		}
	}
}

CUndirectedGraph::CUndirectedGraph(int vParticlesCount, const std::vector<std::vector<int>>& vTargetConstraints, EConstraintsType vConstraintType, const std::vector<int>& vFlags)
{
	_ASSERTE(vTargetConstraints.size() > 0);
	m_ConstrstintsType = vConstraintType;
	m_NodeCount = vTargetConstraints.size();
	m_NodeDegree.resize(m_NodeCount, 0);
	m_AdjacentMatrix = Eigen::MatrixXi(m_NodeCount, m_NodeCount);
	m_AdjacentTable.resize(m_NodeCount);
	std::vector<std::vector<int>> ParticlesConstraintsTable;
	ParticlesConstraintsTable.resize(vParticlesCount);

	for (int i = 0; i < vTargetConstraints.size(); i++)
	{
		std::vector<int> ParticleSet = vTargetConstraints[i];
		for (int k = 0; k < ParticleSet.size(); k++)
		{
			int currentParticleIndex = ParticleSet[k];
			if (vFlags[currentParticleIndex] > 0)
				continue;
			std::vector<int> ConstraintsInCurrentParticle = ParticlesConstraintsTable[currentParticleIndex];
			if (ConstraintsInCurrentParticle.size() != 0)
			{
				for (int j = 0; j < ConstraintsInCurrentParticle.size(); j++)
				{
					addEdge(ConstraintsInCurrentParticle[j], i);
				}
			}

			ParticlesConstraintsTable[currentParticleIndex].push_back(i);
		}
	}
}

//*********************************************************************
//FUNCTION:
void Animation::CUndirectedGraph::addEdge(int vStartNodeIndex, int vEndNodeIndex)
{
	if (vStartNodeIndex < 0 || vStartNodeIndex >= m_NodeCount) throw std::invalid_argument("Start index overflow.");
	if (vEndNodeIndex < 0 || vEndNodeIndex >= m_NodeCount) throw std::invalid_argument("End index overflow.");

	if (m_AdjacentMatrix(vStartNodeIndex, vEndNodeIndex) != 1 && m_AdjacentMatrix(vStartNodeIndex, vEndNodeIndex) != 1)
	{
		m_AdjacentMatrix(vStartNodeIndex, vEndNodeIndex) = 1;
		m_AdjacentMatrix(vEndNodeIndex, vStartNodeIndex) = 1;
		
		m_AdjacentTable[vStartNodeIndex].push_back(vEndNodeIndex);
		m_AdjacentTable[vEndNodeIndex].push_back(vStartNodeIndex);

		m_NodeDegree[vStartNodeIndex]++;
		m_NodeDegree[vEndNodeIndex]++;

		m_EdgeCount++;
	}	
}

//*********************************************************************
//FUNCTION:
void Animation::CUndirectedGraph::deleteEdge(int vStartNodeIndex, int vEndNodeIndex)
{
	if (vStartNodeIndex < 0 || vStartNodeIndex >= m_NodeCount) throw std::invalid_argument("Start index overflow.");
	if (vEndNodeIndex < 0 || vEndNodeIndex >= m_NodeCount) throw std::invalid_argument("End index overflow.");

	if (m_AdjacentMatrix(vStartNodeIndex, vEndNodeIndex) == 0 && m_AdjacentMatrix(vEndNodeIndex, vStartNodeIndex) == 0)
		return;

	m_AdjacentMatrix(vStartNodeIndex, vEndNodeIndex) = 0;
	m_AdjacentMatrix(vEndNodeIndex, vStartNodeIndex) = 0;

	m_NodeDegree[vStartNodeIndex]--;
	m_NodeDegree[vEndNodeIndex]--;

	m_EdgeCount--;

	auto itr = find(m_AdjacentTable[vStartNodeIndex].begin(), m_AdjacentTable[vStartNodeIndex].end(), vEndNodeIndex);
	if (itr != m_AdjacentTable[vStartNodeIndex].end())
	{
		m_AdjacentTable[vStartNodeIndex].erase(itr);
	}

	itr = find(m_AdjacentTable[vEndNodeIndex].begin(), m_AdjacentTable[vEndNodeIndex].end(), vStartNodeIndex);
	if (itr != m_AdjacentTable[vEndNodeIndex].end())
	{
		m_AdjacentTable[vEndNodeIndex].erase(itr);
	}
}

//*********************************************************************
//FUNCTION:
void Animation::CUndirectedGraph::generateShortestDistanceOf(int vNodeIndex)
{
	_ASSERTE(vNodeIndex >= 0 && vNodeIndex < m_NodeCount);

	std::vector<int> DestinationIndex(m_NodeCount, 0);
	for (int i = 0; i < m_NodeCount; i++) DestinationIndex[i] = i;

	std::vector<int> ShortestDistance = findShortestDistanceByDijskra(vNodeIndex, DestinationIndex);
#pragma omp critical
	{
		for (int i = 0; i < m_NodeCount; i++)
		{
			m_AdjacentMatrix(vNodeIndex, DestinationIndex[i]) = ShortestDistance[i];
			m_AdjacentMatrix(DestinationIndex[i], vNodeIndex) = ShortestDistance[i];
		}
	}
}

//*********************************************************************
//FUNCTION:
int Animation::CUndirectedGraph::findShortestDistance(int vSourceNodeIndex, int vDestinationNodeIndex) const
{
	_ASSERTE(vSourceNodeIndex >= 0 && vSourceNodeIndex < m_NodeCount);
	_ASSERTE(vDestinationNodeIndex >= 0 && vDestinationNodeIndex < m_NodeCount);

	int Distance = m_AdjacentMatrix(vSourceNodeIndex, vDestinationNodeIndex);
	if (Distance >= MAX_DISTANCE)
		throw std::runtime_error("The shortest distance from " + std::to_string(vSourceNodeIndex) + " to " + std::to_string(vDestinationNodeIndex) + " haven't been generated.");

	return Distance;
}

//*********************************************************************
//FUNCTION:
std::vector<int> Animation::CUndirectedGraph::findShortestDistance(int vSourceNodeIndex, const std::vector<int>& vDestinationNodeIndices) const
{
	int NumOfDestinationNode = vDestinationNodeIndices.size();
	_ASSERTE(NumOfDestinationNode > 0);

	std::vector<int> ShortestDistance(NumOfDestinationNode, MAX_DISTANCE);
	for (int i = 0; i < NumOfDestinationNode; i++)
	{
		ShortestDistance[i] = findShortestDistance(vSourceNodeIndex, vDestinationNodeIndices[i]);
	}

	return ShortestDistance;
}

//*********************************************************************
//FUNCTION:
std::vector<int> Animation::CUndirectedGraph::findShortestDistanceByDijskra(int vSourceNodeIndex, std::vector<int> vDestinationNodeIndices) const
{
	_ASSERTE(vSourceNodeIndex >= 0 && vSourceNodeIndex < m_NodeCount);

	int DestinationNodeNum = vDestinationNodeIndices.size();
	std::vector<int> ShortestDistance(DestinationNodeNum, MAX_DISTANCE);
	boost::dynamic_bitset<> DestinationSet(m_NodeCount, false);
	for (auto Index : vDestinationNodeIndices)
	{
		_ASSERTE(Index >= 0 && Index < m_NodeCount);
		DestinationSet[Index] = true;
	}

	int FoundCount = 0;
	boost::dynamic_bitset<> VisitedSet(m_NodeCount, false);
	std::priority_queue<Node> PriorytyQueue;
	PriorytyQueue.push(Node(vSourceNodeIndex, 0));
	VisitedSet[vSourceNodeIndex] = true;
	while (!PriorytyQueue.empty())
	{
		Node TempNode = PriorytyQueue.top();
		PriorytyQueue.pop();

		if (DestinationSet[TempNode.Index])
		{
			auto Pos = std::find(vDestinationNodeIndices.begin(), vDestinationNodeIndices.end(), TempNode.Index);
			int Index = std::distance(vDestinationNodeIndices.begin(), Pos);
			ShortestDistance[Index] = TempNode.Distance;
			FoundCount++;
			if (FoundCount >= DestinationNodeNum) break;
		}

		for (int k = 0; k < m_NodeCount; k++)
		{
			if (!VisitedSet[k] && m_AdjacentMatrix(TempNode.Index, k) < MAX_DISTANCE)
			{
				VisitedSet[k] = true;
				PriorytyQueue.push(Node(k, TempNode.Distance + m_AdjacentMatrix(TempNode.Index, k)));
			}
		}
	}

	return ShortestDistance;
}

int Animation::CUndirectedGraph::generateAllMaximalCliquesByBronKerbosch()
{
	m_MaximalCliques.clear();
	std::vector<int> R;
	std::vector<int> P;
	std::vector<int> X;

	for (int i = 0; i < m_NodeCount; i++)
	{
		P.push_back(i);
	}
	std::vector<int> BlackList;
	__BK(R, P, X, BlackList);

	return m_MaximalCliques.size();
}

void Animation::CUndirectedGraph::__BK(std::vector<int> vioR, std::vector<int> vioP, std::vector<int> vioX, std::vector<int>& vblackList)
{
	if (vioP.empty() && vioX.empty())
	{
		if (vioR.size() > m_ColorNumberQ)
		{
			m_OnFindMaxCliqueFunc(vioR, m_ColorNumberQ, m_ConstrstintsType, *this);
			m_MaximalCliques.push_back(vioR);
		}
		return;
	}
	if (vioP.empty())
	{
		return;
	}
	int pivot = vioP[0];
	for (int i = vioP.size() - 1; i >= 0; i--)
	{
		int CurrentNodeIndex = vioP[i];
		std::vector<int> NeighborsOfCurrentNode = m_AdjacentTable[CurrentNodeIndex];

		if (NeighborsOfCurrentNode.size() < m_ColorNumberQ) 
		{
			vblackList.push_back(CurrentNodeIndex);
			continue;
		}

		for (int k = 0; k < NeighborsOfCurrentNode.size(); k++) 
		{
			if (std::find(vblackList.begin(), vblackList.end(), NeighborsOfCurrentNode[k]) != vblackList.end())
			{
				continue;
			}
		}

		if (find(m_AdjacentTable[pivot].begin(), m_AdjacentTable[pivot].end(), CurrentNodeIndex) != m_AdjacentTable[pivot].end() 
			&& CurrentNodeIndex != pivot)
		{
			continue;
		}

		__BK(
			computeUnionOfVectorWithInt(vioR, CurrentNodeIndex),
			computeIntersectionOfTwoVector(vioP, NeighborsOfCurrentNode),
			computeIntersectionOfTwoVector(vioX, NeighborsOfCurrentNode),
			vblackList
		);
		vioP.erase(find(vioP.begin(), vioP.end(), CurrentNodeIndex));
		vioX = computeUnionOfVectorWithInt(vioX, CurrentNodeIndex);
	}
}

std::vector<std::vector<int>> Animation::CUndirectedGraph::ColorGraph()
{
	_ASSERTE(m_NodeCount > 0);
	_ASSERTE(m_ColorNumberQ > 0);

	int stateNotColored = -1;
	int stateNotProcessed = -2;
	std::vector<std::vector<int>> ResultIndex(m_ColorNumberQ, std::vector<int>());
	std::vector<int> ColorOfNode(m_NodeCount, stateNotProcessed);

	int startNodeIndex;
	if (!m_MaximalCliques.empty())
	{
		startNodeIndex = m_MaximalCliques[0][0];
	}
	else
	{
		startNodeIndex = 0;
	}

	std::queue<int> NodeQueue;
	NodeQueue.push(startNodeIndex);
	ColorOfNode[startNodeIndex] = stateNotColored;

	while (!NodeQueue.empty())
	{
		int CurrentNode = NodeQueue.front();
		NodeQueue.pop();

		//Node has been Colored
		if (ColorOfNode[CurrentNode] != -1)
		{
			continue;
		}

		std::vector<int> Neighbors = m_AdjacentTable[CurrentNode];
		std::vector<bool> AvailableColor(m_ColorNumberQ, true);
		//color can't use for current Node
		for (int i = 0; i < Neighbors.size(); i++)
		{
			int NeighborColor = ColorOfNode[Neighbors[i]];
			if (NeighborColor >= 0)
			{
				AvailableColor[NeighborColor] = false;
			}
		}
		//color used by least number of nodes
		int TargetColor = 0;
		for (int i = 0; i < ResultIndex.size(); i++)
		{
			if (ResultIndex[i].size() < ResultIndex[TargetColor].size())
			{
				if (AvailableColor[i])
				{
					TargetColor = i;
				}
			}
		}
		ResultIndex[TargetColor].push_back(CurrentNode);

		//color the node which has least degree first
		sort(Neighbors.begin(), Neighbors.end(), [&](const int& vLeft, const int& vRight) 
		{
			return m_AdjacentTable[vLeft].size() < m_AdjacentTable[vRight].size();
		});
		for (int i = 0; i < Neighbors.size(); i++)
		{
			if (ColorOfNode[Neighbors[i]] == stateNotProcessed)
			{
				NodeQueue.push(Neighbors[i]);
				ColorOfNode[Neighbors[i]] = -1;
			}
		}
	}
	return ResultIndex;
}

//顺序着色
std::vector<std::vector<int>> Animation::CUndirectedGraph::SequentialColorGraph(const std::vector<int>& vOrder)
{
	std::vector<int> NodeColor(m_NodeCount, -1);
	int ColoredNum = 0;
	int K = -1;
	while (ColoredNum < m_NodeCount)
	{
		K++;
		for (int i = 0; i < m_NodeCount; i++)
		{
			int curNode = vOrder[i];
			if (NodeColor[curNode] < 0)
			{
				bool ok = true;
				std::vector<int> Neighbors = m_AdjacentTable[curNode];
				for (int j = 0; j < Neighbors.size(); j++)
				{
					//if neighbor has same color
					if (NodeColor[Neighbors[j]] == K && m_AdjacentMatrix(curNode, Neighbors[j]) == 1)
					{
						ok = false;
						break;
					}
				}

				if (!ok)
					continue;

				NodeColor[curNode] = K;
				ColoredNum++;
			}
		}
	}

	std::vector<std::vector<int>> ResultIndex(K + 1, std::vector<int>());
	for (int i = 0; i < m_NodeCount; i++)
	{
		int CurNodeColor = NodeColor[i];
		ResultIndex[CurNodeColor].push_back(i);
	}
	return ResultIndex;
}

//按照Largest-Degree First顺序的图着色
std::vector<std::vector<int>> Animation::CUndirectedGraph::doWelchPowellColorGraph()
{
	_ASSERTE(m_NodeCount > 0);
	std::vector<int> Nodes(m_NodeCount);
	for (int i = 0; i < m_NodeCount; i++)
	{
		Nodes[i] = i;
	}

	sort(Nodes.begin(), Nodes.end(), [&](const int& vLeft, const int& vRight)
	{
		return m_NodeDegree[vLeft] > m_NodeDegree[vRight];
	});

	return SequentialColorGraph(Nodes);
}

//按照Smallest-Last Ordering顺序的图着色
std::vector<std::vector<int>> Animation::CUndirectedGraph::doSLOColorGraph()
{
	_ASSERTE(m_NodeCount > 0);
	std::vector<int> Nodes;
	int MaxDegree = 0;
	int MinDegree = m_NodeCount;
	for (int i = 0; i < m_NodeCount; i++)
	{
		if (m_NodeDegree[i] > MaxDegree)
			MaxDegree = m_NodeDegree[i];
		if (m_NodeDegree[i] < MinDegree)
			MinDegree = m_NodeDegree[i];
	}

	std::vector<std::list<int>> DegreeMap(MaxDegree+1, std::list<int>());
	std::vector<int> TempDegree(m_NodeCount);
	for (int i = 0; i < m_NodeCount; i++)
	{
		int degree = m_NodeDegree[i];
		TempDegree[i] = degree;
		DegreeMap[degree].push_back(i);
	}

	while (Nodes.size() < m_NodeCount)
	{
		while (DegreeMap[MinDegree].empty())
		{
			MinDegree++;
		}
		int NodeIndex = DegreeMap[MinDegree].back();
		Nodes.insert(Nodes.begin(), NodeIndex);
		DegreeMap[MinDegree].pop_back();

		//将邻居移到度数减一的数组中
		std::vector<int> Neighbors = m_AdjacentTable[NodeIndex];
		for (int i = 0; i < Neighbors.size(); i++)
		{
			if (m_AdjacentMatrix(NodeIndex, Neighbors[i]) == 1)
			{
				int sourceDegree = TempDegree[Neighbors[i]];
				auto it = std::find(DegreeMap[sourceDegree].begin(), DegreeMap[sourceDegree].end(), Neighbors[i]);
				if (it != DegreeMap[sourceDegree].end())
				{
					DegreeMap[sourceDegree].erase(it);
					TempDegree[Neighbors[i]] = sourceDegree - 1;
					_ASSERTE(TempDegree[Neighbors[i]] >= 0);

					DegreeMap[TempDegree[Neighbors[i]]].push_back(Neighbors[i]);
					if (TempDegree[Neighbors[i]] < MinDegree)
						MinDegree = TempDegree[Neighbors[i]];
				}						
			}
		}
	}

	return SequentialColorGraph(Nodes);
}

void Animation::CUndirectedGraph::setNeighborOfNode(int vNodeIndex, const std::vector<int>& vNeighborIndices)
{
	for (auto OldNeighborIndex : m_AdjacentTable[vNodeIndex])
	{
		std::vector<int>::iterator itr = std::find(m_AdjacentTable[OldNeighborIndex].begin(), m_AdjacentTable[OldNeighborIndex].end(), vNodeIndex);
		m_AdjacentTable[OldNeighborIndex].erase(itr);

		m_AdjacentMatrix(OldNeighborIndex, vNodeIndex) = 0;
		m_AdjacentMatrix(vNodeIndex, OldNeighborIndex) = 0;
		m_NodeDegree[vNodeIndex]--;
		m_NodeDegree[OldNeighborIndex]--;
	}
	m_AdjacentTable[vNodeIndex].assign(vNeighborIndices.begin(), vNeighborIndices.end());
	for (auto NeighborIndex : vNeighborIndices)
	{
		std::vector<int>::iterator itr = std::find(m_AdjacentTable[NeighborIndex].begin(), m_AdjacentTable[NeighborIndex].end(), vNodeIndex);
		if (itr == m_AdjacentTable[NeighborIndex].end())
		{
			m_AdjacentTable[NeighborIndex].push_back(vNodeIndex);

			m_AdjacentMatrix(NeighborIndex, vNodeIndex) = 1;
			m_AdjacentMatrix(vNodeIndex, NeighborIndex) = 1;
			m_NodeDegree[vNodeIndex]++;
			m_NodeDegree[NeighborIndex]++;
		}
	}
}
