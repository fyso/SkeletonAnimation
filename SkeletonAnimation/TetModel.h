#pragma once
#include <vector>

namespace Animation
{
	class CTetModel
	{
	public:
		CTetModel() = default;
		virtual ~CTetModel() = default;

		bool initializeMesh(const std::vector<int>& vParticleIndices);

		int getTetSize() const { return m_TetIndices.size(); }
		const std::vector<int>& getTet(int vIndex) { _ASSERTE(vIndex < m_TetIndices.size()); return m_TetIndices[vIndex]; }
		int getEdgeSize() const { return m_TetEdges.size(); }
		const std::pair<int, int>& getEdge(unsigned int vIndex) { _ASSERTE(vIndex < m_TetEdges.size()); return m_TetEdges[vIndex]; }

	private:
		void __addTet(const std::vector<int>& vIndices);
		void __addEdge(const std::vector<int>& vIndices);

		std::vector<std::vector<int>> m_TetIndices;
		std::vector<std::pair<int, int>> m_TetEdges;
	};
}