#pragma once
#include "Common.h"
#include <boost/shared_ptr.hpp>

namespace Animation
{
	class CUndirectedGraph
	{
	public:
		explicit CUndirectedGraph(int vNodeCount);
		explicit CUndirectedGraph(
			int vParticlesCount, 
			const std::vector<std::vector<int>>& vTargetConstraints, 
			EConstraintsType vConstraintType, 
			const std::vector<int>& vFlags
		);
		virtual ~CUndirectedGraph() = default;

		void addEdge(int vStartNodeIndex, int vEndNodeIndex);
		void deleteEdge(int vStartNodeIndex, int vEndNodeIndex);

		void generateShortestDistanceOf(int vNodeIndex);
		int	 generateAllMaximalCliquesByBronKerbosch();
		int  findShortestDistance(int vSourceNodeIndex, int vDestinationNodeIndex) const;
		std::vector<int> findShortestDistance(int vSourceNodeIndex, const std::vector<int>& vDestinationNodeIndices) const;
		std::vector<int> findShortestDistanceByDijskra(int vSourceNodeIndex, std::vector<int> vDestinationNodeIndices) const;
		
		std::vector<std::vector<int>> ColorGraph();
		std::vector<std::vector<int>> doWelchPowellColorGraph();
		std::vector<std::vector<int>> doSLOColorGraph();
		std::vector<std::vector<int>> SequentialColorGraph(const std::vector<int>& vOrder);

		void setMaximalCliques(const std::vector<std::vector<int>> vInput) { m_MaximalCliques = vInput; }
		void setColorNumberQ(const int vInput) { m_ColorNumberQ = vInput; }
		void setOnFindMaxCliqueFunc(std::function<void(const std::vector<int>&, int, EConstraintsType, CUndirectedGraph&)> vInput) { m_OnFindMaxCliqueFunc = vInput; }
		void setNeighborOfNode(int vNodeIndex, const std::vector<int>& vNeighborIndices);

		const std::vector<std::vector<int>>& getMaximalCliques() const { return m_MaximalCliques; }
		const std::vector<int>& getNeighborOfNode(int vIndex)  const { return m_AdjacentTable[vIndex]; }
		const std::vector<std::vector<int>> getAdjacentTable() const { return m_AdjacentTable; }
		int   getColorNumberQ() const { return m_ColorNumberQ; }

#ifdef _UNIT_TEST
		int findShortestDistanceByDijskra(int vSourceNodeIndex, int vDestinationNodeIndex) const
		{
			std::vector<int> ShortestDistance = findShortestDistanceByDijskra(vSourceNodeIndex, std::vector<int>(1, vDestinationNodeIndex));
			_ASSERTE(ShortestDistance.size() == 1);
			return ShortestDistance[0];
		}
#endif // _UNIT_TEST

	private:
		int m_NodeCount = -1;
		int m_ColorNumberQ = 0;
		int m_EdgeCount = 0;
		std::vector<int> m_NodeDegree;
		Eigen::MatrixXi m_AdjacentMatrix;
		EConstraintsType m_ConstrstintsType;
		std::vector<std::vector<int>> m_MaximalCliques;
		std::vector<std::vector<int>> m_AdjacentTable;

		std::function<void(
			const std::vector<int>&, 
			int, EConstraintsType, 
			CUndirectedGraph&
			)> m_OnFindMaxCliqueFunc;

		void __BK(
			std::vector<int> vioR, 
			std::vector<int> vioP, 
			std::vector<int> vioX, 
			std::vector<int>& vblackList
		);
	};
}