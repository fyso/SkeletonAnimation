#pragma once
#include <common/Singleton.h>
#include "SimulationModel.h"
#include "ParticleSource.h"
#include "TimeStepController.h"
#include "ParticleLayer.h"
#include "SkeletonController.h"
#include "CUDACommon.h"

namespace Animation
{
	class CSimulation : public hiveDesignPattern::CSingleton<CSimulation>
	{
	public:
		virtual ~CSimulation() = default;

		void initialize(CParticleSource* vParticleSource, CParticleLayer* vParticleLayer, CSkeletonController* vSkeletonController);
		std::vector<Eigen::Vector3d> update();

	private:
		CSimulation() = default;
		friend class hiveDesignPattern::CSingleton<CSimulation>;
		void __initParameters();
		void __initSimulationModel();
		void __updateBoneParticlePosition();

		boost::shared_ptr<CSimulationModel> m_pSimulationModel = nullptr;
		boost::shared_ptr<CTimeStepController> m_pTimeStepController = nullptr;
		boost::shared_ptr<CParticleSource> m_pParticleSource = nullptr;
		boost::shared_ptr<CParticleLayer> m_pParticleLayer = nullptr;
		boost::shared_ptr<CSkeletonController> m_pSkeletonController = nullptr;

		// simulation parameter
		Eigen::Vector3d m_Gravity = { 0.0, 0.0, 0.0 };
		double m_DampFactor = 0.0;
		int m_IterationCount = 0;
		double m_TimeStep = 0.0;
		double m_StopWatchValue = 0.0;
		int m_SimulationCountPerFrame = 0;
		std::string m_CacheFilePath = "./SimulateModel";
	};
}