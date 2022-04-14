#pragma once
#include "CUDACommon.h"
#include "ParticleData.h"
#include <Eigen/Dense>

namespace Animation
{
	class CShapeMatchingConstraint
	{
		friend class boost::serialization::access;
	public:
		CUDA_FUNC CShapeMatchingConstraint() = default;
		CUDA_FUNC explicit CShapeMatchingConstraint(int vParticleCount) : m_ParticleCount(vParticleCount) {}
		CUDA_FUNC ~CShapeMatchingConstraint() = default;

		__host__ bool initializeConstraint(const std::vector<CParticleData>& vParticleData, const std::vector<int>& vParticleIndices, const std::vector<int>& vClusterCount, const std::vector<double>& vStiffness)
		{
			for (int i = 0; i < m_ParticleCount; i++)
			{
				m_ParticleIndices[i] = vParticleIndices[i];
				m_X0[i] = vParticleData[m_ParticleIndices[i]].getPosition0();
				m_Weight[i] = vParticleData[m_ParticleIndices[i]].getInverseMass();
				m_ClusterCount[i] = vClusterCount[i];
				m_Stiffness[i] = vStiffness[i];
			}

			const bool Result = __initShapeMatchingConstraint();
			return Result;
		}

		CUDA_FUNC int getParticleCount() const { return m_ParticleCount; }
		CUDA_FUNC int* getParticleIndices() { return m_ParticleIndices; }
		CUDA_FUNC double* getStiffness() { return m_Stiffness; }
		CUDA_FUNC double* getWeight() { return m_Weight; }
		CUDA_FUNC int* getClusterCount() { return m_ClusterCount; }
		CUDA_FUNC const SVector3d& getRestCm() const { return m_RestCm; }
		CUDA_FUNC const SMatrix3d& getInvRestMat() const { return m_InvRestMat; }
		CUDA_FUNC SVector3d* getPosition0() { return m_X0; }

	private:
		__host__ bool __initShapeMatchingConstraint()
		{
			m_InvRestMat.setIdentity();

			// center of mass
			Eigen::Vector3d RestCm = Eigen::Vector3d::Zero();
			double WeightSum = 0.0;
			for (int i = 0; i < m_ParticleCount; i++)
			{
				const double Weight = 1.0 / (m_Weight[i] + DBL_EPSILON);
				Eigen::Vector3d X0(m_X0[i].x, m_X0[i].y, m_X0[i].z);
				RestCm += X0 * Weight;
				WeightSum += Weight;
			}

			if (WeightSum < DBL_EPSILON)
				return false;

			RestCm /= WeightSum;
			m_RestCm = { RestCm[0], RestCm[1], RestCm[2] };

			// A is a constant matrix
			Eigen::Matrix3d A;
			A.setZero();
			for (int i = 0; i < m_ParticleCount; i++)
			{
				Eigen::Vector3d X0(m_X0[i].x, m_X0[i].y, m_X0[i].z);
				const Eigen::Vector3d Q = X0 - RestCm;
				const double Weight = 1.0 / (m_Weight[i] + DBL_EPSILON);
				const double X2 = Weight * Q[0] * Q[0];
				const double Y2 = Weight * Q[1] * Q[1];
				const double Z2 = Weight * Q[2] * Q[2];
				const double XY = Weight * Q[0] * Q[1];
				const double XZ = Weight * Q[0] * Q[2];
				const double YZ = Weight * Q[1] * Q[2];

				A(0, 0) += X2; A(0, 1) += XY; A(0, 2) += XZ;
				A(1, 0) += XY; A(1, 1) += Y2; A(1, 2) += YZ;
				A(2, 0) += XZ; A(2, 1) += YZ; A(2, 2) += Z2;
			}

			double Det = A.determinant();
			if (fabs(Det) > DBL_EPSILON)
			{
				Eigen::Matrix3d InvRestMat = A.inverse();
				m_InvRestMat(0, 0) = InvRestMat(0, 0); m_InvRestMat(0, 1) = InvRestMat(0, 1); m_InvRestMat(0, 2) = InvRestMat(0, 2);
				m_InvRestMat(1, 0) = InvRestMat(1, 0); m_InvRestMat(1, 1) = InvRestMat(1, 1); m_InvRestMat(1, 2) = InvRestMat(1, 2);
				m_InvRestMat(2, 0) = InvRestMat(2, 0); m_InvRestMat(2, 1) = InvRestMat(2, 1); m_InvRestMat(2, 2) = InvRestMat(2, 2);
				return true;
			}
			return false;
		}

		int m_ParticleCount = -1;
		int m_ParticleIndices[27] = { 0 };
		double m_Stiffness[27] = { 0.0 };
		double m_Weight[27] = { 0.0 };
		int m_ClusterCount[27] = { 0 };
		SVector3d m_RestCm;  //center of mass in rest pose
		SMatrix3d m_InvRestMat;
		SVector3d m_X0[27];
		SVector3d m_Correct[27];

		template<typename Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			ar & m_ParticleCount;
			ar & m_ParticleIndices;
			ar & m_Stiffness;
			ar & m_Weight;
			ar & m_ClusterCount;
			ar & m_RestCm;
			ar & m_InvRestMat;
			ar & m_X0;
			ar & m_Correct;
		}
	};
}