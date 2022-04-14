#pragma once
#include "Common.h"

namespace Animation
{
#define CBaseGraphPtr std::shared_ptr<pbd::CBaseGraph>
#define CBaseGraphMaker std::make_shared<pbd::CBaseGraph>

	class CBaseGraph
	{
	public:
		CBaseGraph() = default;
		CBaseGraph(int vNodeCount) :m_NodeCount(vNodeCount) {}
		CBaseGraph(const CBaseGraph& vOther) :m_NodeCount(vOther.m_NodeCount) {}

		virtual void addEdgeV(int vStartNodeIndex, int vEndNodeIndex) = 0;
		virtual void deleteEdgeV(int vStartNodeIndex, int vEndNodeIndex) = 0;
		virtual std::set<int> getNeighborOfNodeV(int vIndex) const = 0;
		virtual bool hasEdgeV(int vStartNodeIndex, int vEndNodeIndex) = 0;
		virtual void clearAndReinitializeGraphDataV(int vNodeCount) = 0;

		/*get&set*/
		void setNodeCount(int vNodeCount) { m_NodeCount = vNodeCount; }
		int getNodeCount() const { return m_NodeCount; }

	protected:
		int m_NodeCount = -1;
	};
}

