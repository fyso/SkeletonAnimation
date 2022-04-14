#pragma once
#include <array>

class CConstraintBase
{
public:
	virtual int getParticleCount() const = 0;
	virtual const std::vector<int> getParticleIndices() const = 0;
};

