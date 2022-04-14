#include "TetModel.h"

using namespace Animation;

//*********************************************************************
//FUNCTION:
bool CTetModel::initializeMesh(const std::vector<int>& vParticleIndices)
{
	int ParticleCount = vParticleIndices.size();
	_ASSERTE(ParticleCount % 4 == 0);

	for (int i = 0; i < ParticleCount; i += 4)
	{
		std::vector<int> Indices(4);
		for (int k = 0; k < 4; k++)
			Indices[k] = vParticleIndices[i + k];

		__addTet(Indices);
		__addEdge(Indices);
	}

	return (m_TetIndices.size() > 0 && m_TetEdges.size() > 0) ? true : false;
}

//*********************************************************************
//FUNCTION:
void CTetModel::__addTet(const std::vector<int>& vIndices)
{
	std::vector<int> TetIndices(vIndices);
	m_TetIndices.push_back(TetIndices);
}

//*********************************************************************
//FUNCTION:
void CTetModel::__addEdge(const std::vector<int>& vIndices)
{
	// tet edge indices: {0,1, 0,2, 0,3, 1,2, 1,3, 2,3}
	const int Edges[12] = { vIndices[0], vIndices[1],
						vIndices[0], vIndices[2],
						vIndices[0], vIndices[3],
						vIndices[1], vIndices[2],
						vIndices[1], vIndices[3],
						vIndices[2], vIndices[3] };

	for (int k = 0; k < 6; k++)
	{
		const int Vertex0 = Edges[k * 2];
		const int Vertex1 = Edges[k * 2 + 1];

		bool IsEdgeExisted = false;
		for (auto Edge : m_TetEdges)
		{
			if ((Edge.first == Vertex0 && Edge.second == Vertex1) || (Edge.first == Vertex1 && Edge.second == Vertex0))
			{
				IsEdgeExisted = true;
				break;
			}
		}

		if (!IsEdgeExisted)
			m_TetEdges.push_back({ Vertex0, Vertex1 });
	}
}