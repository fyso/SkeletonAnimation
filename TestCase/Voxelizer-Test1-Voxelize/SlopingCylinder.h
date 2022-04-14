#pragma once
#include "Cylinder.h"
#include "SlopingModel.h"

class CSlopingCylinder : public CCylinder, public CSlopingModel
{
public:
	CSlopingCylinder(const Animation::SAABB& vSceneAABB, float vVoxelSize, const glm::vec3& vCenter, float vRadius, float vHalfHeight, const glm::vec3 vRotation) :
		CCylinder(vSceneAABB, vVoxelSize, vCenter, vRadius, vHalfHeight), CSlopingModel(vRotation) {}
	virtual ~CSlopingCylinder() = default;

protected:
	bool _testV(const glm::vec3& vPoint) const override
	{
		glm::vec4 Position = m_RotationMatZ * m_RotationMatY * m_RotationMatX * glm::vec4(vPoint, 1.0);

		const glm::vec3 PointOnXZPlane{ Position.x, 0, Position.z };
		const glm::vec3 CenterOnXZPlane{ m_Center.x,0, m_Center.z };
		const bool IsInCylinderOfXZPlane = distance(PointOnXZPlane, CenterOnXZPlane) <= m_Radius;
		const bool IsInCylinderOfYAxis = abs(Position.y - m_Center.y) <= m_HalfHeight;
		return IsInCylinderOfXZPlane && IsInCylinderOfYAxis;
	}
};