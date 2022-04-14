#pragma once
#include "CUDACommon.h"
#include "ParticleData.h"
#include "Common.h"

namespace Animation
{
	class CProjectionConstraint
	{
		friend class boost::serialization::access;
	public:
		CUDA_FUNC CProjectionConstraint() = default;
		CUDA_FUNC ~CProjectionConstraint() = default;

		__host__ bool initializeConstraint(const std::vector<int>& vParticles)
		{
			m_ParticleCount = vParticles.size();
			for (int i = 0; i < m_ParticleCount; i++)
				m_ParticleIndices[i] = vParticles[i];

			return true;
		}

		CUDA_FUNC int getParticleCount() const { return m_ParticleCount; }
		CUDA_FUNC int* getParticleIndices() { return m_ParticleIndices; }

	private:
		int m_ParticleCount = -1;
		int m_ParticleIndices[50] = { -1 };

		template<typename Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			ar & m_ParticleIndices;
			ar & m_ParticleCount;
		}
	};
}