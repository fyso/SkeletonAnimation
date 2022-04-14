#include "ColorableGraph.h"
#include <fstream>
#include <iostream>

Animation::CColorableGraph::CColorableGraph() : CBaseGraph(0)
{
	m_ColorNumberQ = 0;
	m_AdjacentTable = std::make_shared<AdjacentTable>(0);
}

Animation::CColorableGraph::CColorableGraph(int vNodeCount, int vColourNumber) : CBaseGraph(vNodeCount)
{
	m_ColorNumberQ = vColourNumber;
	m_AdjacentTable = std::make_shared<AdjacentTable>(vNodeCount);
}

Animation::CColorableGraph::CColorableGraph(const CColorableGraph & vOther) : CBaseGraph(vOther)
{
	m_AdjacentTable = vOther.m_AdjacentTable;
	m_ColorNumberQ = vOther.m_ColorNumberQ;
}

void Animation::CColorableGraph::addEdgeV(int vStartNodeIndex, int vEndNodeIndex)
{
	_ASSERT(vStartNodeIndex >= 0 && vStartNodeIndex < m_NodeCount);
	_ASSERT(vEndNodeIndex >= 0 && vEndNodeIndex < m_NodeCount);

	if (!hasEdgeV(vStartNodeIndex, vEndNodeIndex))
	{
		boost::add_edge(vStartNodeIndex, vEndNodeIndex, 1, *m_AdjacentTable);
	}
	else
	{
		unsigned int Weight = getEdgeWeight(vStartNodeIndex, vEndNodeIndex);
		setEdgeWeight(vStartNodeIndex, vEndNodeIndex, Weight + 1);
	}
}

void Animation::CColorableGraph::deleteEdgeV(int vStartNodeIndex, int vEndNodeIndex)
{
	_ASSERT(vStartNodeIndex >= 0 && vStartNodeIndex < m_NodeCount);
	_ASSERT(vEndNodeIndex >= 0 && vEndNodeIndex < m_NodeCount);

	if (hasEdgeV(vStartNodeIndex, vEndNodeIndex))
	{
		boost::remove_edge(vStartNodeIndex, vEndNodeIndex, *m_AdjacentTable);
	}
}

std::set<int> Animation::CColorableGraph::getNeighborOfNodeV(int vIndex) const
{
	_ASSERT(vIndex >= 0 && vIndex < m_NodeCount);

	std::set<int> Neighbors;
	IndexMap Indices = boost::get(vertex_index, *m_AdjacentTable);

	AdjacencyVertexIterator ai;
	AdjacencyVertexIterator ai_end;
	for (tie(ai, ai_end) = adjacent_vertices(vIndex, *m_AdjacentTable); ai != ai_end; ++ai)
	{
		Neighbors.insert(static_cast<int>(Indices[*ai]));
	}
	return Neighbors;
}

bool Animation::CColorableGraph::hasEdgeV(int vStartNodeIndex, int vEndNodeIndex)
{
	_ASSERT(vStartNodeIndex >= 0 && vStartNodeIndex < m_NodeCount);
	_ASSERT(vEndNodeIndex >= 0 && vEndNodeIndex < m_NodeCount);

	if (vStartNodeIndex == vEndNodeIndex) return false;
	std::set<int> Neigbors = getNeighborOfNodeV(vStartNodeIndex);
	return Neigbors.find(vEndNodeIndex) != Neigbors.end();
}

void Animation::CColorableGraph::clearAndReinitializeGraphDataV(int vNodeCount)
{
	m_NodeCount = vNodeCount;
	m_AdjacentTable->clear();
	m_AdjacentTable = std::make_shared<AdjacentTable>(vNodeCount);
}

std::vector<std::vector<int>> Animation::CColorableGraph::colorGraphWithSLO()
{
	m_CoupledEdgeWeight = 0;

	std::vector<int> Nodes;
	int MaxDegree = 0;
	int MinDegree = m_NodeCount;
	for (int i = 0; i < m_NodeCount; i++)
	{
		int curDegree = getNodeDegree(i);
		if (curDegree > MaxDegree)
			MaxDegree = curDegree;
		if (curDegree < MinDegree)
			MinDegree = curDegree;
	}

	std::vector<std::list<int>> DegreeMap(MaxDegree + 1, std::list<int>());
	std::vector<int> TempDegree(m_NodeCount);
	for (int i = 0; i < m_NodeCount; i++)
	{
		int degree = getNodeDegree(i);
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
		std::set<int> Neighbors = getNeighborOfNodeV(NodeIndex);
		for (auto idx : Neighbors)
		{
			int sourceDegree = TempDegree[idx];
			auto it = std::find(DegreeMap[sourceDegree].begin(), DegreeMap[sourceDegree].end(), idx);
			if (it != DegreeMap[sourceDegree].end())
			{
				DegreeMap[sourceDegree].erase(it);
				TempDegree[idx] = sourceDegree - 1;
				_ASSERTE(TempDegree[idx] >= 0);

				DegreeMap[TempDegree[idx]].push_back(idx);
				if (TempDegree[idx] < MinDegree)
					MinDegree = TempDegree[idx];
			}
		}
	}
	std::vector<std::vector<int>> Result = __sequentialColorGraphFixedGroup(Nodes);

	//输出算法负载
	std::cout << __computeEdgeCouplingWeight(Result) << std::endl;

	return Result;
}

std::vector<std::vector<int>> Animation::CColorableGraph::colorGraphWithLDF()
{
	std::vector<int> Nodes(m_NodeCount);
	std::iota(Nodes.begin(), Nodes.end(), 0);

	sort(Nodes.begin(), Nodes.end(), [&](const int& vLeft, const int& vRight)
	{
		return getNodeDegree(vLeft) > getNodeDegree(vRight);
	});

	std::vector<std::vector<int>> Result = __sequentialColorGraphFixedGroup(Nodes);

	//输出算法负载
	//std::cout << __computeEdgeCouplingWeight(Result) << std::endl;

	return Result;
}

std::vector<std::vector<int>> Animation::CColorableGraph::colorGraphWithClique()
{
	m_CoupledEdgeWeight = 0;

	m_ColorNodeTable.resize(m_ColorNumberQ);
	m_NodeColorTable.resize(m_NodeCount, -1);
	std::sort(m_Cliques.begin(), m_Cliques.end(), 
		[&](const std::vector<int>& vFirst, const std::vector<int>& vSecond)
		{ 
			return vFirst.size() > vSecond.size(); 
		});
	std::vector<std::pair<int, int>> Colors(m_ColorNumberQ, std::pair<int, int>());

	for (int i = 0; i < Colors.size(); i++)
	{
		Colors[i].first = i;
		Colors[i].second = 0;
	}

	//按团着色
	for (std::vector<int>& Clique : m_Cliques)
	{
		std::sort(Colors.begin(), Colors.end(), 
			[&](const std::pair<int, int>& vFirst, const std::pair<int, int>& vSecond)
			{ 
				return vFirst.second < vSecond.second; 
			});
		__colorOneClique(Clique, Colors);
	}

	//把剩下的不在团里的节点着色
	__colorNodesOutOfStructure(Colors);

	//输出算法负载
	//std::cout << __computeEdgeCouplingWeight(m_ColorNodeTable) << std::endl;
	return m_ColorNodeTable;
}

std::vector<std::vector<int>> Animation::CColorableGraph::colorGraphWithLEF()
{
	m_ColorNodeTable.resize(m_ColorNumberQ);
	m_NodeColorTable.resize(m_NodeCount, -1);
	std::vector<std::pair<int, int>> Colors(m_ColorNumberQ, std::pair<int, int>());
	for (int i = 0; i < Colors.size(); i++)
	{
		Colors[i].first = i;
		Colors[i].second = 0;
	}

	std::vector<std::pair<int, int>> AllEdges;
	std::pair<AdjacencyEdgeIterator, AdjacencyEdgeIterator> EdgeStartEndIt;
	EdgeStartEndIt = edges(*m_AdjacentTable);
	for (AdjacencyEdgeIterator It = EdgeStartEndIt.first; It != EdgeStartEndIt.second; It++)
	{
		AllEdges.push_back(std::make_pair(It->m_source, It->m_target));
	}
	std::sort(AllEdges.begin(), AllEdges.end(),
		[&](const std::pair<int, int>& vFirst, const std::pair<int, int>& vSecond)
		{
			return  getEdgeWeight(vFirst.first, vFirst.second) > getEdgeWeight(vSecond.first, vSecond.second);
		});

	for (int i = 0; i < AllEdges.size(); i++)
	{
		std::sort(Colors.begin(), Colors.end(),
			[&](const std::pair<int, int>& vFirst, const std::pair<int, int>& vSecond)
			{
				return vFirst.second < vSecond.second;
			});

		int StartNode = AllEdges[i].first;
		int EndNode = AllEdges[i].second;
		if (m_NodeColorTable[StartNode] == -1 && m_NodeColorTable[EndNode] == -1)
		{
			__colorOneNode(StartNode, Colors);
			__colorOneNode(EndNode, Colors);
		}
		else if (m_NodeColorTable[StartNode] != -1 && m_NodeColorTable[EndNode] == -1)
		{
			__colorOneNode(EndNode, Colors);
		}
		else if (m_NodeColorTable[StartNode] == -1 && m_NodeColorTable[EndNode] != -1)
		{
			__colorOneNode(StartNode, Colors);
		}
		else
		{
			continue;
		}
	}

	__colorNodesOutOfStructure(Colors);

	//输出算法负载
	std::cout << __computeEdgeCouplingWeight(m_ColorNodeTable) << std::endl;
	return m_ColorNodeTable;
}

void Animation::CColorableGraph::__colorOneClique(std::vector<int>& vClique, std::vector<std::pair<int, int>>& voColors)
{
	std::vector<int> Nodes(vClique);
	int NumColor = m_ColorNumberQ;
	int NumNode = Nodes.size();

	//全不同色着色
	if (NumColor >= NumNode)
	{
		for (int i = 0; i < NumNode; i++)
		{
			m_NodeColorTable[Nodes[i]] = voColors[i].first;
			m_ColorNodeTable[voColors[i].first].push_back(Nodes[i]);
			voColors[i].second++;
		}
	}
	else
	{
		//拿到所有边
		std::vector<std::pair<int, int>> Edges;
		int TempCount = 0;
		for (int i = 0; i < NumNode; i++)
		{
			for (int k = i + 1; k < NumNode; k++)
			{
				Edges.push_back(std::make_pair(Nodes[i], Nodes[k]));
			}
		}

		//边按权重升序排序
		sort(Edges.begin(), Edges.end(), [&](const std::pair<int, int>& vLeft, const std::pair<int, int>& vRight)
			{
				return getEdgeWeight(vLeft.first, vLeft.second) > getEdgeWeight(vRight.first, vRight.second);
			});

		//团内节点分组
		std::vector<std::vector<int>> GroupInClique;
		for (int i = 0; i < NumNode - NumColor; i++)
		{
			int StartNode = Edges[i].first;
			int EndNode = Edges[i].second;

			bool IsFounded = false;
			for (std::vector<int>& Group : GroupInClique)
			{
				if (std::find(Group.begin(), Group.end(), StartNode) != Group.end())
				{
					Group.push_back(EndNode);
					IsFounded = true;
				}
				else if (std::find(Group.begin(), Group.end(), EndNode) != Group.end())
				{
					Group.push_back(StartNode);
					IsFounded = true;
				}
			}
			if (!IsFounded)
			{
				std::vector<int> TempGroup;
				TempGroup.push_back(StartNode);
				TempGroup.push_back(EndNode);
				GroupInClique.push_back(TempGroup);
			}
		}

		for (int i = 0; i < NumNode; i++)
		{
			bool IsFounded = false;
			for (std::vector<int>& Group : GroupInClique)
			{
				if (std::find(Group.begin(), Group.end(), Nodes[i]) != Group.end())
				{
					IsFounded = true;
				}
			}
			if (!IsFounded)
			{
				std::vector<int> TempGroup;
				TempGroup.push_back(Nodes[i]);
				GroupInClique.push_back(TempGroup);
			}
		}
		
		//每一组给颜色
		sort(GroupInClique.begin(), GroupInClique.end(), [&](const std::vector<int>& vLeft, const std::vector<int>& vRight)
			{
				return vLeft.size() > vRight.size();
			});

		for (int i = 0; i < NumColor; i++)
		{
			for (int Node : GroupInClique[i])
			{
				m_ColorNodeTable[voColors[i].first].push_back(Node);
				m_NodeColorTable[Node] = voColors[i].first;
			}
			voColors[i].second += GroupInClique[i].size();
		}
	}
}

float Animation::CColorableGraph::__computeEdgeCouplingWeight(const std::vector<std::vector<int>>& vColorNodeTable)
{
	int CouplingValue = 0;
	int NumOfCoupling = 0;
	for (std::vector<int> Group : vColorNodeTable)
	{
		for (int Node1 : Group)
		{
			for (int Node2 : Group)
			{
				if(Node2 == Node1 || !hasEdgeV(Node1, Node2))
					continue;
				CouplingValue += getEdgeWeight(Node1, Node2);
				//NumOfCoupling++;
			}
		}
	}
	return float(CouplingValue / 2.0f) / float(getTotalEdgeWeight());
	//return float(NumOfCoupling);
}

void Animation::CColorableGraph::__colorOneNode(int NodeIndex, std::vector<std::pair<int, int>>& voColors)
{
	std::vector<int> AllNeighbors = getNeighborOfNode(NodeIndex);
	std::vector<int> NeighborsWithColors;

	for (int i = 0; i < AllNeighbors.size(); i++)
	{
		if (m_NodeColorTable[AllNeighbors[i]] == -1)
		{
			AllNeighbors[i] = -1;
		}
	}

	for (int TempNeighbor : AllNeighbors)
	{
		if (TempNeighbor != -1)
		{
			NeighborsWithColors.push_back(TempNeighbor);
		}
	}

	if (NeighborsWithColors.size() == 0)
	{
		m_NodeColorTable[NodeIndex] = voColors[0].first;
		m_ColorNodeTable[voColors[0].first].push_back(NodeIndex);
		voColors[0].second++;
	}
	else if (NeighborsWithColors.size() < m_ColorNumberQ)
	{
		for (std::pair<int, int>& Color : voColors)
		{
			bool IsUsed = false;
			for (int Neighbor : NeighborsWithColors)
			{
				if (m_NodeColorTable[Neighbor] == Color.first)
				{
					IsUsed = true;
					break;
				}
			}
			if (!IsUsed)
			{
				m_NodeColorTable[NodeIndex] = Color.first;
				m_ColorNodeTable[Color.first].push_back(NodeIndex);
				Color.second++;
				break;
			}
		}
	}
	else
	{
		std::vector<std::pair<int, int>> Edges;
		for (int i = 0; i < NeighborsWithColors.size(); i++)
		{
			Edges.push_back(std::make_pair(NodeIndex, NeighborsWithColors[i]));
		}

		sort(Edges.begin(), Edges.end(), [&](const std::pair<int, int>& vLeft, const std::pair<int, int>& vRight)
			{
				return getEdgeWeight(vLeft.first, vLeft.second) > getEdgeWeight(vRight.first, vRight.second);
			});

		int TempNode = Edges[0].second;
		m_NodeColorTable[NodeIndex] = m_NodeColorTable[TempNode];
		m_ColorNodeTable[m_NodeColorTable[NodeIndex]].push_back(NodeIndex);

		m_CoupledEdgeWeight += getEdgeWeight(NodeIndex, TempNode);

		for (std::pair<int, int>& Color : voColors)
		{
			if (Color.first == m_NodeColorTable[NodeIndex])
			{
				Color.second++;
			}
		}
	}
}

void Animation::CColorableGraph::__colorNodesOutOfStructure(std::vector<std::pair<int, int>>& voColors)
{
	std::vector<int> NodesOutOfStructures;

	for (int i = 0; i < m_NodeColorTable.size(); i++)
	{
		if (m_NodeColorTable[i] == -1)
		{
			NodesOutOfStructures.push_back(i);
		}
	}

	//对单个顶点着色
	for (int NodeIndex : NodesOutOfStructures)
	{
		std::sort(voColors.begin(), voColors.end(), [&](const std::pair<int, int>& vFirst, const std::pair<int, int>& vSecond)->bool { return vFirst.second < vSecond.second; });
		__colorOneNode(NodeIndex, voColors);
	}
}

void Animation::CColorableGraph::searchMaximalCliquesByBronKerbosch(std::function<void(const std::vector<int>&)> vOnFindMaximalCliques)
{
	if(vOnFindMaximalCliques != nullptr)
		m_EventFindMaximalCliques.connect(vOnFindMaximalCliques);

	std::vector<int> R;
	std::vector<int> X;

	std::vector<int> P(m_NodeCount);
	std::iota(P.begin(), P.end(), 0);
	sort(P.begin(), P.end(), [&](const int& vLeft, const int& vRight)
	{
		return getNodeDegree(vLeft) < getNodeDegree(vRight);
	});

	std::vector<int> BlackList;
	__BronKerboshClique(R, P, X, BlackList);

	m_EventFindMaximalCliques.disconnect_all_slots();
}

bool Animation::CColorableGraph::searchCliqueInChildGraph(std::vector<int> vNodes)
{
	std::vector<int> R, X, BlackList;

	if (__searchClique(R, vNodes, X, BlackList))
		return true;

	return false;
}

int Animation::CColorableGraph::getTotalEdgeWeight()
{
	int WeightSum = 0;
	std::pair<AdjacencyEdgeIterator, AdjacencyEdgeIterator> EdgeStartEndIt;
	EdgeStartEndIt = edges(*m_AdjacentTable);
	for (AdjacencyEdgeIterator It = EdgeStartEndIt.first; It != EdgeStartEndIt.second; It++)
	{
		EdgeProperty EdgePro = *static_cast<EdgeProperty*>(It->get_property());
		WeightSum += EdgePro.m_value;
	}
	return WeightSum;
}

bool Animation::CColorableGraph::testConflictInSameColorGroup(const std::vector<int>& vNodes)
{
	for (int i = 0; i < vNodes.size()-1; i++)
	{
		for (int t = i + 1; t < vNodes.size(); t++)
		{
			if (hasEdgeV(vNodes[i], vNodes[t]))
				return true;
		}
	}
	return false;
}

void Animation::CColorableGraph::writeColorGroupToFile(const std::vector<std::vector<int>>& vGraph, const std::string& vFileName)
{
	std::ofstream FileOut(vFileName, std::fstream::out);
	if (FileOut)
	{
		for (int i = 0; i < vGraph.size(); i++)
		{
			FileOut << i << " ";
			for (int Node : vGraph[i])
			{
				FileOut << Node << " ";
			}
			FileOut << ";" << std::endl;
		}
	}
	FileOut.close();
}

bool Animation::CColorableGraph::operator==(const CColorableGraph & vOther)
{
	if (vOther.m_ColorNumberQ != m_ColorNumberQ) return false;
	if (vOther.m_NodeCount != m_NodeCount)    return false;

	for (int i = 0; i < m_NodeCount; i++)
	{
		if (vOther.getNeighborOfNodeV(i) != getNeighborOfNodeV(i)) return false;
	}

	return true;
}

void Animation::CColorableGraph::__BronKerboshClique(std::vector<int> vR, std::vector<int> vP, std::vector<int> vX, std::vector<int>& voBlackList)
{
	if (vP.empty() && vX.empty())
	{
		if(!m_EventFindMaximalCliques.empty())
			m_EventFindMaximalCliques(vR);

		bool CanPutIntoClique = true;
		int TargetIndex = -1;
		for (int Node : vR)
		{
			if (m_NodeCliqueMap.count(Node) != 0)
			{
				TargetIndex = m_NodeCliqueMap[Node];
				if (vR.size() <= m_Cliques[TargetIndex].size())
				{
					CanPutIntoClique = false;
					break;
				}
			}
		}
		if (CanPutIntoClique)
		{
			if (TargetIndex == -1)
			{
				m_Cliques.push_back(vR); 
				for (int Node : vR)
				{
					m_NodeCliqueMap[Node] = m_Cliques.size() - 1;
				}
			}
			else
			{
				for (int Node : m_Cliques[TargetIndex])
				{
					m_NodeCliqueMap.erase(Node);
				}

				m_Cliques[TargetIndex] = vR; 

				for (int Node : vR)
				{
					m_NodeCliqueMap[Node] = TargetIndex;
				}
			}
		}
	}
	if (vP.empty())
	{
		return;
	}
	int Pivot = vP[0];
	std::vector<int> NeighborsOfPivot = getNeighborOfNode(Pivot);
	for (int i = vP.size() - 1; i >= 0; i--)
	{
		int CurrentNodeIndex = vP[i];

		if (find(voBlackList.begin(), voBlackList.end(), CurrentNodeIndex) != voBlackList.end())
		{
			continue;
		}

		std::vector<int> NeighborsOfCurrentNode = getNeighborOfNode(CurrentNodeIndex);

		//if (NeighborsOfCurrentNode.size() <= m_ColorNumberQ - 1 && m_ColorNumberQ != 0)
		//{
		//	voBlackList.push_back(CurrentNodeIndex);
		//	continue;
		//}

		if (find(NeighborsOfPivot.begin(), NeighborsOfPivot.end(), CurrentNodeIndex) != NeighborsOfPivot.end()
			&& CurrentNodeIndex != Pivot)
		{
			continue;
		}

		__BronKerboshClique(
			computeUnionOfVectorWithInt(vR, CurrentNodeIndex),
			computeIntersectionOfTwoVector(vP, NeighborsOfCurrentNode),
			computeIntersectionOfTwoVector(vX, NeighborsOfCurrentNode),
			voBlackList
		);

		vP.erase(find(vP.begin(), vP.end(), CurrentNodeIndex));
		vX = computeUnionOfVectorWithInt(vX, CurrentNodeIndex);
	}

}

//bool Animation::CColorableGraph::__searchClique(std::set<int> vR, std::set<int> vP, std::set<int> vX, std::set<int>& voBlackList)
//{
//	if (vP.empty() && vX.empty())
//	{
//		if (vR.size() > m_ColorNumberQ || m_ColorNumberQ == 0)
//		{
//			return true;
//		}
//		return false;
//	}
//	if (vP.empty())
//	{
//		return false;
//	}
//	const std::set<int>::iterator Pivot = vP.begin();
//	for (std::set<int>::reverse_iterator IteratorOfP = vP.rbegin(); IteratorOfP != vP.rend(); IteratorOfP++)
//	{
//		int CurrentNodeIndex = *IteratorOfP;
//		std::set<int> NeighborsOfCurrentNode = getNeighborOfNodeV(CurrentNodeIndex);
//		std::set<int> NeighborsOfPivot = getNeighborOfNodeV(*Pivot);
//
//		if (NeighborsOfCurrentNode.size() <= m_ColorNumberQ - 1 && m_ColorNumberQ != 0)
//		{
//			voBlackList.insert(CurrentNodeIndex);
//			continue;
//		}
//
//		if (find(NeighborsOfPivot.begin(), NeighborsOfPivot.end(), CurrentNodeIndex) != NeighborsOfPivot.end()
//			&& CurrentNodeIndex != *Pivot)
//		{
//			continue;
//		}
//
//		vR.insert(CurrentNodeIndex);
//
//		std::set<int> NewP, NewX;
//		std::set_intersection(vP.begin(), vP.end(), NeighborsOfCurrentNode.begin(), NeighborsOfCurrentNode.end(), std::inserter(NewP, NewP.begin()));
//		std::set_intersection(vX.begin(), vX.end(), NeighborsOfCurrentNode.begin(), NeighborsOfCurrentNode.end(), std::inserter(NewX, NewX.begin()));
//
//		if (__searchClique(vR, NewP, NewX, voBlackList))
//			return true;
//
//		vP.erase(CurrentNodeIndex);
//		vX.insert(CurrentNodeIndex);
//		vR.erase(CurrentNodeIndex);
//
//	}
//	return false;
//}

bool Animation::CColorableGraph::__searchClique(std::vector<int> vR, std::vector<int> vP, std::vector<int> vX, std::vector<int>& voBlackList)
{
	if (vP.empty() && vX.empty())
	{
		if (vR.size() > m_ColorNumberQ || m_ColorNumberQ == 0)
		{
			return true;
		}
		return false;
	}
	if (vP.empty())
	{
		return false;
	}
	int Pivot = vP[0];
	std::vector<int> NeighborsOfPivot = getNeighborOfNode(Pivot);
	for (int i = vP.size() - 1; i >= 0; i--)
	{
		int CurrentNodeIndex = vP[i];
		std::vector<int> NeighborsOfCurrentNode = getNeighborOfNode(CurrentNodeIndex);

		if (NeighborsOfCurrentNode.size() <= m_ColorNumberQ - 1 && m_ColorNumberQ != 0)
		{
			voBlackList.push_back(CurrentNodeIndex);
			continue;
		}

		if (find(NeighborsOfPivot.begin(), NeighborsOfPivot.end(), CurrentNodeIndex) != NeighborsOfPivot.end()
			&& CurrentNodeIndex != Pivot)
		{
			continue;
		}

		if (__searchClique(computeUnionOfVectorWithInt(vR, CurrentNodeIndex),
			computeIntersectionOfTwoVector(vP, NeighborsOfCurrentNode),
			computeIntersectionOfTwoVector(vX, NeighborsOfCurrentNode),
			voBlackList))
		{
			return true;
		}

		vP.erase(find(vP.begin(), vP.end(), CurrentNodeIndex));
		vX = computeUnionOfVectorWithInt(vX, CurrentNodeIndex);
	}
	return false;
}

std::vector<std::vector<int>> Animation::CColorableGraph::__sequentialColorGraph(const std::vector<int>& vOrder)
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
				std::set<int> Neighbors = getNeighborOfNodeV(curNode);
				for (auto neighbor : Neighbors)
				{
					if (NodeColor[neighbor] == K)
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

std::vector<std::vector<int>> Animation::CColorableGraph::__sequentialColorGraphFixedGroup(const std::vector<int>& vOrder)
{
	std::vector<int> NodeColor(m_NodeCount, -1);

	srand(time(NULL));
	for (int i = 0; i < m_NodeCount; i++)
	{
		int curNode = vOrder[i];
		if (NodeColor[curNode] < 0)
		{
			std::vector<int> BlackList;
			std::set<int> Neighbors = getNeighborOfNodeV(curNode);
			for (auto neighbor : Neighbors)
			{
				if (NodeColor[neighbor] != -1)
					BlackList.push_back(NodeColor[neighbor]);
			}
			std::vector<int> WhiteList;
			for (int k = 0; k < m_ColorNumberQ; k++)
			{
				if (std::find(BlackList.begin(), BlackList.end(), k) == BlackList.end())
				{
					WhiteList.push_back(k);
				}
			}

			if (WhiteList.empty())
			{
				int MinWeight = 999;
				int MinWeightColor = 0;
				for (int Node : Neighbors)
				{
					if(NodeColor[Node] == -1)
						continue;
					
					int CurrEdgeWeight = getEdgeWeight(curNode, Node);
					if (CurrEdgeWeight < MinWeight)
					{
						MinWeight = CurrEdgeWeight;
						MinWeightColor = NodeColor[Node];
					}
				}

				NodeColor[curNode] = MinWeightColor;
				for (auto neighbor : Neighbors)
				{
					if (NodeColor[neighbor] == NodeColor[curNode])
						m_CoupledEdgeWeight+=getEdgeWeight(neighbor, curNode);
				}
			}
			else
			{
				NodeColor[curNode] = WhiteList[std::rand() % WhiteList.size()];
			}
		}
	}

	std::vector<std::vector<int>> ResultIndex(m_ColorNumberQ, std::vector<int>());
	for (int i = 0; i < m_NodeCount; i++)
	{
		int CurNodeColor = NodeColor[i];
		ResultIndex[CurNodeColor].push_back(i);
	}
	return ResultIndex;
}

unsigned int Animation::CColorableGraph::getEdgeWeight(int vStartNodeIndex, int vEndNodeIndex)
{
	AdjacentTable::edge_descriptor Edge = edge(vStartNodeIndex, vEndNodeIndex, *m_AdjacentTable).first;
	EdgeProperty EdgePro = *static_cast<EdgeProperty*>(Edge.get_property());
	return EdgePro.m_value;
}

void Animation::CColorableGraph::setEdgeWeight(int vStartNodeIndex, int vEndNodeIndex, unsigned int vWeight)
{
	AdjacentTable::edge_descriptor Edge = edge(vStartNodeIndex, vEndNodeIndex, *m_AdjacentTable).first;
	EdgeProperty* EdgePro = static_cast<EdgeProperty*>(Edge.get_property());
	EdgePro->m_value = vWeight;
}

std::vector<int> Animation::CColorableGraph::getNeighborOfNode(int vIndex) const
{
	_ASSERT(vIndex >= 0 && vIndex < m_NodeCount);

	std::vector<int> Neighbors;
	IndexMap Indices = boost::get(vertex_index, *m_AdjacentTable);

	AdjacencyVertexIterator ai;
	AdjacencyVertexIterator ai_end;
	for (tie(ai, ai_end) = adjacent_vertices(vIndex, *m_AdjacentTable); ai != ai_end; ++ai)
	{
		Neighbors.push_back(static_cast<int>(Indices[*ai]));
	}
	return Neighbors;
}