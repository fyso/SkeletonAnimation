#pragma once
#include "SimulationModel.h"

namespace Animation
{
	class CTimeStepController
	{
	public:
		CTimeStepController(int vIterationCount, double vTimeStep, double vDampFactor);
		virtual ~CTimeStepController();

		void init(CSimulationModel& vSimulationModel, int vOriginParticleCount);
		bool step();
		void updateBoneParticlesWithCuda(const std::vector<int>& vIndices, const std::vector<SVector3d>& vPos);
		std::vector<Eigen::Vector3d> getPositions();
		void resetParticleData(CSimulationModel& vSimulationModel);

	private:
		void __freeCudaMemory();
		std::vector<int> m_DistanceConstraintOffset;
		std::vector<int> m_VolumeConstraintOffset;
		std::vector<int> m_ShapeMatchingConstraintOffset;

		int m_IterationCount = 0;
		double m_TimeStep = 0.0;
		double m_DampFactor = 0.0;
		int m_OriginParticleCount = 0;
		int m_ParticleCount = 0;
		int m_DistanceConstraintCount = 0;
		int m_DistanceProjectionConstraintCount = 0;
		int m_VolumeConstraintCount = 0;
		int m_VolumeProjectionConstraintCount = 0;
		int m_ShapeMatchingConstraintCount = 0;
		int m_ShapeMatchingProjectionConstraintCount = 0;
		int m_MaxThreadsPerBlock = 0;

		CParticleData *m_dParticleData = nullptr;
		CDistanceConstraint *m_dDistanceConstraints = nullptr;
		CProjectionConstraint *m_dDistanceProjectionConstraints = nullptr;
		CVolumeConstraint *m_dVolumeConstraints = nullptr;
		CProjectionConstraint *m_dVolumeProjectionConstraints = nullptr;
		CShapeMatchingConstraint *m_dShapeMatchingConstraints = nullptr;
		CProjectionConstraint *m_dShapeMatchingProjectionConstraints = nullptr;

		int *m_dParticleRegionCount = nullptr;
	};
}