#pragma once
#include "Geometry.h"

class CCuboid : public IGeometry
{
public:
	CCuboid(const Animation::SAABB& vAABB, float vVoxelSize, const glm::vec3& vMinPoint, const glm::vec3& vMaxPoint) :
		IGeometry(vAABB, vVoxelSize), m_MinPoint(vMinPoint), m_MaxPoint(vMaxPoint) {}
	virtual ~CCuboid() = default;

protected:
	bool _testV(const glm::vec3& vPoint) const override
	{
		return ((vPoint.x <= m_MaxPoint.x) && (vPoint.x >= m_MinPoint.x)
			&& (vPoint.y <= m_MaxPoint.y) && (vPoint.y >= m_MinPoint.y)
			&& (vPoint.z <= m_MaxPoint.z) && (vPoint.z >= m_MinPoint.z));
	}

	glm::vec3 m_MinPoint;
	glm::vec3 m_MaxPoint;
};