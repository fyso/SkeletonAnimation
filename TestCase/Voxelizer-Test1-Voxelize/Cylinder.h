#pragma once
#include "Geometry.h"
#include <glm/ext.hpp>

class CCylinder : public IGeometry
{
public:
	CCylinder(const Animation::SAABB& vSceneAABB, float vVoxelSize, const glm::vec3& vCenter, float vRadius, float vHalfHeight)
		: IGeometry(vSceneAABB, vVoxelSize), m_Center(vCenter), m_Radius(vRadius), m_HalfHeight(vHalfHeight) {}
	virtual ~CCylinder() = default;

protected:
	bool _testV(const glm::vec3& vPoint) const override
	{
		const glm::vec3 PointOnXZPlane{ vPoint.x, 0, vPoint.z };
		const glm::vec3 CenterOnXZPlane{ m_Center.x,0, m_Center.z };
		const bool IsInCylinderOfXZPlane = distance(PointOnXZPlane, CenterOnXZPlane) <= m_Radius;
		const bool IsInCylinderOfYAxis = abs(vPoint.y - m_Center.y) <= m_HalfHeight;
		return IsInCylinderOfXZPlane && IsInCylinderOfYAxis;
	}

	glm::vec3 m_Center;
	float m_Radius = 0;
	float m_HalfHeight = 0;
};