#pragma once
#include "Common.h"
#include "BaseGraph.h"
#include <numeric>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/signals2.hpp>

using namespace boost;
typedef property<edge_weight_t, unsigned int> EdgeProperty;
typedef adjacency_list<hash_setS, vecS, undirectedS, boost::no_property, EdgeProperty> AdjacentTable;
typedef property_map<AdjacentTable, edge_weight_t>::type WeightMap;
typedef property_map<AdjacentTable, vertex_index_t>::type IndexMap;
typedef graph_traits<AdjacentTable>::adjacency_iterator AdjacencyVertexIterator;
typedef graph_traits<AdjacentTable>::edge_iterator  AdjacencyEdgeIterator;

namespace Animation
{
#define CColorableGraphPtr std::shared_ptr<Animation::CColorableGraph>
#define CColorableGraphMaker std::make_shared<Animation::CColorableGraph>
	class CColorableGraph : public CBaseGraph
	{
	public:
		CColorableGraph();
		CColorableGraph(int vNodeCount, int vColourNumber = 0);
		CColorableGraph(const CColorableGraph& vOther);

		/*虚方法*/
		virtual void addEdgeV(int vStartNodeIndex, int vEndNodeIndext) override;
		virtual void deleteEdgeV(int vStartNodeIndex, int vEndNodeIndex) override;
		virtual std::set<int> getNeighborOfNodeV(int vIndex) const override;
		virtual bool hasEdgeV(int vStartNodeIndex, int vEndNodeIndex) override;
		virtual void clearAndReinitializeGraphDataV(int vNodeCount) override;

		/*着色相关方法*/
		std::vector<std::vector<int>> colorGraphWithSLO();
		std::vector<std::vector<int>> colorGraphWithLDF();
		std::vector<std::vector<int>> colorGraphWithClique();
		std::vector<std::vector<int>> colorGraphWithLEF();
		void searchMaximalCliquesByBronKerbosch(std::function<void(const std::vector<int>&)> vOnFindMaximalCliques = nullptr);
		bool searchCliqueInChildGraph(std::vector<int> vNodes);
		int getTotalEdgeWeight();

		/*测试相关*/
		bool testConflictInSameColorGroup(const std::vector<int>& vNodes);
		void writeColorGroupToFile(const std::vector<std::vector<int>>& vGraph, const std::string& vFileName);

		/*get&set*/
		void  setColorNumberQ(int vColorNumber) { m_ColorNumberQ = vColorNumber; }
		const std::vector<std::vector<int>>& getClique() const { return m_Cliques; }
		int getEdgeCount() const { return boost::num_edges(*m_AdjacentTable); }
		unsigned int getEdgeWeight(int vStartNodeIndex, int vEndNodeIndex);
		void setEdgeWeight(int vStartNodeIndex, int vEndNodeIndex, unsigned int vWeight);
		int getNodeDegree(int vNodeIdx) const { return boost::degree(vNodeIdx, *m_AdjacentTable); }
		const std::vector<std::vector<int>>& getCliques() const { return m_Cliques; }
		std::vector<int> getNeighborOfNode(int vIndex) const;

		bool operator==(const CColorableGraph& vOther);

	private:
		int m_ColorNumberQ = 0;
		int m_CoupledEdgeWeight = 0;
		std::vector<std::vector<int>> m_Cliques;
		std::shared_ptr<AdjacentTable> m_AdjacentTable;
		boost::signals2::signal<void(const std::vector<int>&)> m_EventFindMaximalCliques;

		float __computeEdgeCouplingWeight(const std::vector<std::vector<int>>& vColorNodeTable);
		void __colorOneNode(int NodeIndex, std::vector<std::pair<int, int>>& voColors);
		void __colorNodesOutOfStructure(std::vector<std::pair<int, int>>& voColors);
		void __colorOneClique(std::vector<int>& vClique, std::vector<std::pair<int, int>>& voColors);
		std::vector<std::vector<int>> __sequentialColorGraph(const std::vector<int>& vOrder);
		std::vector<std::vector<int>> __sequentialColorGraphFixedGroup(const std::vector<int>& vOrder);
		void __BronKerboshClique(std::vector<int> vR, std::vector<int> vP, std::vector<int> vX, std::vector<int>& voBlackList);
		bool __searchClique(std::vector<int> vR, std::vector<int> vP, std::vector<int> vX, std::vector<int>& voBlackList);

		//临时变量
		std::vector<int>			  m_NodeColorTable;
		std::vector<std::vector<int>> m_ColorNodeTable;
		std::map<int, int>			  m_NodeCliqueMap;

		/*序列化*/
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & m_Cliques;
		}
	};
}

