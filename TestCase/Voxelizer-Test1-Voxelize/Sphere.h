#pragma once
#include "Geometry.h"
#include <glm/ext.hpp>

class CSphere : public IGeometry
{
public:
	CSphere(const Animation::SAABB& vAABB, float vVoxelSize, const glm::vec3& vCenter, float vRadius)
		: IGeometry(vAABB, vVoxelSize), m_Center(vCenter), m_Radius(vRadius) {}
	~CSphere() = default;

protected:
	bool _testV(const glm::vec3& vPoint) const override
	{
		return distance(m_Center, vPoint) <= m_Radius;
	}

private:
	glm::vec3 m_Center;
	float m_Radius = 0;
};