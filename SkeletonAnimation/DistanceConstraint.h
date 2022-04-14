#pragma once
#include "CUDACommon.h"
#include "ParticleData.h"

namespace Animation
{
	class CDistanceConstraint
	{
		friend class boost::serialization::access;
	public:
		CUDA_FUNC CDistanceConstraint() = default;
		CUDA_FUNC ~CDistanceConstraint() = default;

		__host__ bool initializeConstraint(const std::vector<CParticleData>& vParticleData, const std::pair<int, int>& vParticlePair, double vStiffness)
		{
			m_Stiffness = vStiffness;

			m_ParticleIndices[0] = vParticlePair.first;
			m_ParticleIndices[1] = vParticlePair.second;

			SVector3d X1_0 = vParticleData[vParticlePair.first].getPosition0();
			SVector3d X2_0 = vParticleData[vParticlePair.second].getPosition0();

			m_RestDistance = (X2_0 - X1_0).norm();
			if (m_RestDistance < DBL_EPSILON)
				return false;

			return true;
		}

		CUDA_FUNC int* getParticleIndices() { return m_ParticleIndices; }
		CUDA_FUNC double getRestDistance() const { return m_RestDistance; }
		CUDA_FUNC double getStiffness() { return m_Stiffness; }

	private:
		int m_ParticleIndices[2] = { 0 };
		double m_RestDistance = 0.0;
		double m_Stiffness = 0.0;

		template<typename Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			ar & m_ParticleIndices;
			ar & m_RestDistance;
			ar & m_Stiffness;
		}
	};
}