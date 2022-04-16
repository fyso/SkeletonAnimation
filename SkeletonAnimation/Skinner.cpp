#include "Skinner.h"
#include "AnimationConfig.h"
#include <iostream>

using namespace Animation;

CSkinner::CSkinner(graphics::CModel *vModel, CParticleSource *vParticleSource) :
	m_pModel(vModel), m_pParticleSource(vParticleSource)
{
	m_AngleThreshold = CAnimationConfig::getInstance()->getAttribute<double>(KEY_WORDS::SKINNING_ANGLE_TOLERANCE);

	__initMeshVertices();
}

CSkinner::~CSkinner()
{
	m_pParticleSource = nullptr;
}

//*********************************************************************
//FUNCTION:
void CSkinner::draw(graphics::CShader vShader)
{
	m_pModel->draw(vShader);
}

//*********************************************************************
//FUNCTION:
void CSkinner::updateAndDraw(graphics::CShader vShader, const std::vector<Eigen::Vector3d>& vPositions, bool vOutPutData)
{
	__updateVertices(vPositions);

	if (vOutPutData)
	{
		//std::ofstream FileOut("./animation output/FatGuyJump_GPU_Vertex_" + std::to_string(Frame++) + ".txt", std::fstream::out);
		//const int MeshCount = m_MeshVertices.size();
		//for (int i = 0; i < MeshCount; i++)
		//{
		//	const int VerticesCount = m_MeshVertices[i].size();
		//	for (int k = 0; k < VerticesCount; k++)
		//	{
		//		Eigen::Vector3d Pos = m_MeshVertices[i][k].Position;
		//		FileOut << Pos.x() << " " << Pos.y() << " " << Pos.z() << std::endl;
		//	}
		//	FileOut << "END" << std::endl;
		//}
		//FileOut.close();
	}

	m_pModel->draw(vShader);
}

//*********************************************************************
//FUNCTION:
void CSkinner::__initMeshVertices()
{
	m_MeshVertices.clear();

	const int MeshCount = m_pModel->getMumOfMeshes();
	m_MeshVertices.resize(MeshCount);
	const double ParticleDiameter = m_pParticleSource->getParticleRadius() * 2.0;
	for (int i = 0; i < MeshCount; i++)
	{
		auto& Mesh = const_cast<graphics::CMesh&>(m_pModel->getMeshesAt(i));
		const int VerticesCount = Mesh.getNumOfVertices();
		m_MeshVertices[i].resize(VerticesCount);
#pragma omp parallel for
		for (int k = 0; k < VerticesCount; k++)
		{
			graphics::SVertex Vertex = Mesh.getVertexAt(k);
			Eigen::Vector3d Position = Eigen::Vector3d(Vertex.Position.x, Vertex.Position.y, Vertex.Position.z);

			SMeshVertex MeshVertex;
			MeshVertex.Position = Position;
			MeshVertex.RelatedParticleIndices = m_pParticleSource->findNearestVertex(Position);
			MeshVertex.OriginPosition = (Position - m_pParticleSource->getParticlePos(MeshVertex.RelatedParticleIndices[0])) / ParticleDiameter;

			m_MeshVertices[i][k] = MeshVertex;
		}
	}
}

//*********************************************************************
//FUNCTION:
void CSkinner::__updateVertices(const std::vector<Eigen::Vector3d>& vPositions)
{
	//TODO: use CUDA to implement skinning
	const int MeshCount = m_MeshVertices.size();
	std::vector<std::vector<glm::vec3>> Pos(MeshCount);
	for (int i = 0; i < MeshCount; i++)
	{
		const int VerticesCount = m_MeshVertices[i].size();
		Pos[i].resize(VerticesCount);
#pragma omp parallel for
		for (int k = 0; k < VerticesCount; k++)
		{
			std::vector<Eigen::Vector3d> ParticlePos;
			for (auto Index : m_MeshVertices[i][k].RelatedParticleIndices)
				ParticlePos.push_back(vPositions[Index]);

			Eigen::Vector3d OriginPosition = m_MeshVertices[i][k].OriginPosition;
			m_MeshVertices[i][k].Position =
				ParticlePos[0] * (1.0 - OriginPosition.x()) * (1.0 - OriginPosition.y()) * (1.0 - OriginPosition.z()) +
				ParticlePos[1] * (OriginPosition.x()) * (1.0 - OriginPosition.y()) * (1.0 - OriginPosition.z()) +
				ParticlePos[2] * (1.0 - OriginPosition.x()) * (1.0 - OriginPosition.y()) * (OriginPosition.z()) +
				ParticlePos[3] * (OriginPosition.x()) * (1.0 - OriginPosition.y()) * (OriginPosition.z()) +
				ParticlePos[4] * (1.0 - OriginPosition.x()) * (OriginPosition.y()) * (1.0 - OriginPosition.z()) +
				ParticlePos[5] * (OriginPosition.x()) * (OriginPosition.y()) * (1.0 - OriginPosition.z()) +
				ParticlePos[6] * (1.0 - OriginPosition.x()) * (OriginPosition.y()) * (OriginPosition.z()) +
				ParticlePos[7] * (OriginPosition.x()) * (OriginPosition.y()) * (OriginPosition.z());

			Pos[i][k] = glm::vec3(m_MeshVertices[i][k].Position[0], m_MeshVertices[i][k].Position[1], m_MeshVertices[i][k].Position[2]);
		}
	}

	m_pModel->updateVertexPosition(Pos, m_AngleThreshold);
}