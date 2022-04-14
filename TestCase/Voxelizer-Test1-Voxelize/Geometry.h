#pragma once
#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include "Common.h"

class IGeometry
{
public:
	IGeometry(const Animation::SAABB& vAABB, float vVoxelSize);
	virtual ~IGeometry() = default;

	void generateVoxelizationResult();
	const boost::dynamic_bitset<>& getVoxelizationResult() const { return m_VoxelSet; }

protected:
	virtual bool _testV(const glm::vec3& vPoint) const = 0;

	boost::dynamic_bitset<> m_VoxelSet;

private:
	glm::vec3 __transformIndexToPos(const glm::ivec3& vIndex) const;
	int __transformIndexToInt(const glm::ivec3& vIndex) const;

	glm::ivec3 m_Resolution;
	float m_VoxelSize = 0;
	Animation::SAABB m_ModelAABB;
};