#pragma once
#include "Cuboid.h"
#include "SlopingModel.h"

class CSlopingCuboid : public CCuboid, public CSlopingModel
{
public:
	CSlopingCuboid(const Animation::SAABB& vAABB, float vVoxelSize, const glm::vec3& vMinPoint, const glm::vec3& vMaxPoint, const glm::vec3 vRotation) :
		CCuboid(vAABB, vVoxelSize, vMinPoint, vMaxPoint), CSlopingModel(vRotation) {}

protected:
	bool _testV(const glm::vec3& vPoint) const override
	{
		glm::vec4 Position = m_RotationMatZ * m_RotationMatY * m_RotationMatX * glm::vec4(vPoint, 1.0);

		return ((Position.x <= m_MaxPoint.x) && (Position.x >= m_MinPoint.x)
			&& (Position.y <= m_MaxPoint.y) && (Position.y >= m_MinPoint.y)
			&& (Position.z <= m_MaxPoint.z) && (Position.z >= m_MinPoint.z));
	}
};