#pragma once
#include "CUDACommon.h"
#include "ParticleData.h"

namespace Animation
{
	class CVolumeConstraint
	{
		friend class boost::serialization::access;
	public:
		CUDA_FUNC CVolumeConstraint() = default;
		CUDA_FUNC ~CVolumeConstraint() = default;

		__host__ bool initializeConstraint(const std::vector<CParticleData>& vParticleData, const std::vector<int>& vParticles, double vStiffness)
		{
			m_Stiffness = vStiffness;
			const int ParticleCount = vParticles.size();
			_ASSERTE(ParticleCount == 4);

			std::vector<SVector3d> Pos(ParticleCount);
			for (int i = 0; i < ParticleCount; i++)
			{
				m_ParticleIndices[i] = vParticles[i];
				Pos[i] = vParticleData[vParticles[i]].getPosition0();
			}

			m_RestVolume = fabs(1.0 / 6.0 * (Pos[3] - Pos[0]).dot((Pos[2] - Pos[0]).cross(Pos[1] - Pos[0])));

			return true;
		}

		CUDA_FUNC int* getParticleIndices() { return m_ParticleIndices; }
		CUDA_FUNC double getRestVolume() const { return m_RestVolume; }
		CUDA_FUNC double getStiffness() { return m_Stiffness; }

	private:
		int m_ParticleIndices[4] = { 0 };
		double m_RestVolume = 0.0;
		double m_Stiffness = 0.0;

		template <typename Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			ar & m_ParticleIndices;
			ar & m_RestVolume;
			ar & m_Stiffness;
		}

	};
}