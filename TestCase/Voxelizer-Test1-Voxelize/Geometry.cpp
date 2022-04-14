#include "pch.h"
#include "Geometry.h"

IGeometry::IGeometry(const Animation::SAABB& vAABB, float vVoxelSize) : m_VoxelSize(vVoxelSize), m_ModelAABB(vAABB)
{
	m_Resolution.x = static_cast<int>(round((vAABB.MaxCoordinate.x - vAABB.MinCoordinate.x) / vVoxelSize));
	m_Resolution.y = static_cast<int>(round((vAABB.MaxCoordinate.y - vAABB.MinCoordinate.y) / vVoxelSize));
	m_Resolution.z = static_cast<int>(round((vAABB.MaxCoordinate.z - vAABB.MinCoordinate.z) / vVoxelSize));

	m_VoxelSet.resize(m_Resolution.x * m_Resolution.y * m_Resolution.z, false);
}

//*********************************************************************
//FUNCTION:
void IGeometry::generateVoxelizationResult()
{
	for (auto i = 0; i < m_Resolution.x; ++i)
		for (auto k = 0; k < m_Resolution.y; ++k)
			for (auto m = 0; m < m_Resolution.z; ++m)
			{
				glm::vec3 Index(i, k, m);
				if (_testV(__transformIndexToPos(Index)))
				{
					m_VoxelSet.set(__transformIndexToInt(Index), true);
				}
			}
}

//*********************************************************************
//FUNCTION:
glm::vec3 IGeometry::__transformIndexToPos(const glm::ivec3& vIndex) const
{
	glm::vec3 Pos;
	Pos.x = vIndex.x * m_VoxelSize + m_VoxelSize / 2 + m_ModelAABB.MinCoordinate.x;
	Pos.y = vIndex.y * m_VoxelSize + m_VoxelSize / 2 + m_ModelAABB.MinCoordinate.y;
	Pos.z = vIndex.z * m_VoxelSize + m_VoxelSize / 2 + m_ModelAABB.MinCoordinate.z;
	return Pos;
}

//*********************************************************************
//FUNCTION:
int IGeometry::__transformIndexToInt(const glm::ivec3& vIndex) const
{
	return vIndex.y * (m_Resolution.x * m_Resolution.z) + vIndex.z * m_Resolution.x + vIndex.x;
}