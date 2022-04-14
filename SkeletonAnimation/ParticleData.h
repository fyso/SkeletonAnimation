#pragma once
#include "CUDACommon.h"
#include "Common.h"

namespace Animation
{
	class CParticleData
	{
		friend class boost::serialization::access;
	public:
		CUDA_FUNC CParticleData() = default;
		CUDA_FUNC ~CParticleData() = default;

		CUDA_FUNC void setInfo(const SVector3d& vPos, const SVector3d& vGravity)
		{
			m_X0 = vPos;
			m_X = vPos;
			m_OldX = vPos;
			m_LastX = vPos;
			m_Mass = 1.0;
			m_InvMass = 1.0;
			m_V = SVector3d(0.0);
			m_A = vGravity;
		}
		CUDA_FUNC void setMass(double vMass)
		{
			m_Mass = vMass;
			if (vMass != 0.0)
				m_InvMass = static_cast<double>(1.0) / vMass;
			else
				m_InvMass = 0.0;
		}
		CUDA_FUNC void resetState()
		{
			m_X = m_X0;
			m_OldX = m_X;
			m_LastX = m_X;
			m_V = SVector3d(0.0);
		}
		CUDA_FUNC void setPosition(const SVector3d& vPosition) { m_X = vPosition; }
		CUDA_FUNC void setLastPosition(const SVector3d& vPosition) { m_LastX = vPosition; }
		CUDA_FUNC void setOldPosition(const SVector3d& vPosition) { m_OldX = vPosition; }
		CUDA_FUNC void setVelocity(const SVector3d& vVelocity) { m_V = vVelocity; }
		CUDA_FUNC void setAcceleration(const SVector3d& vAcceleration) { m_A = vAcceleration; }
		
		CUDA_FUNC double getMass() const { return m_Mass; }
		CUDA_FUNC double getInverseMass() const { return m_InvMass; }
		CUDA_FUNC const SVector3d& getPosition0() const { return m_X0; }
		CUDA_FUNC const SVector3d& getPosition() const { return m_X; }
		CUDA_FUNC const SVector3d& getLastPosition() const { return m_LastX; }
		CUDA_FUNC const SVector3d& getOldPosition() const { return m_OldX; }
		CUDA_FUNC const SVector3d& getVelocity() const { return m_V; }
		CUDA_FUNC const SVector3d& getAcceleration() const { return m_A; }
		CUDA_FUNC SVector3d& fetchPosition0() { return m_X0; }
		CUDA_FUNC SVector3d& fetchPosition() { return m_X; }
		CUDA_FUNC SVector3d& fetchLastPosition() { return m_LastX; }
		CUDA_FUNC SVector3d& fetchOldPosition() { return m_OldX; }
		CUDA_FUNC SVector3d& fetchVelocity() { return m_V; }
		CUDA_FUNC SVector3d& fetchAcceleration() { return m_A; }

	private:
		// If the mass is zero, the particle is static
		double m_Mass;
		double m_InvMass; // the inverse of mass

		// Dynamic state
		SVector3d m_X0;
		SVector3d m_X;
		SVector3d m_V;
		SVector3d m_A;
		SVector3d m_OldX;
		SVector3d m_LastX;

		template <typename Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			ar & m_Mass;
			ar & m_InvMass;
			ar & m_X0;
			ar & m_X;
			ar & m_V;
			ar & m_OldX;
			ar & m_LastX;
		}

	};
}