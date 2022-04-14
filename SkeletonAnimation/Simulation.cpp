#include "Simulation.h"
#include "AnimationConfig.h"
#include "Common.h"
#include <fstream>

//*********************************************************************
//FUNCTION:
void Animation::CSimulation::initialize(CParticleSource* vParticleSource, CParticleLayer* vParticleLayer, CSkeletonController* vSkeletonController)
{
	m_SimulationCountPerFrame = CAnimationConfig::getInstance()->getAttribute<int>(KEY_WORDS::SIMULATION_COUNT_PER_FRAME);
	m_pParticleSource = boost::shared_ptr<CParticleSource>(vParticleSource);
	m_pParticleLayer = boost::shared_ptr<CParticleLayer>(vParticleLayer);
	m_pSkeletonController = boost::shared_ptr<CSkeletonController>(vSkeletonController);

	__initParameters();
	__initSimulationModel();

	if (m_pTimeStepController == nullptr)
	{
		m_pTimeStepController = boost::shared_ptr<CTimeStepController>(new CTimeStepController(m_IterationCount, m_TimeStep, m_DampFactor));
		m_pTimeStepController->init(*m_pSimulationModel, m_pParticleSource->getParticleCount());
	}
}

//*********************************************************************
//FUNCTION:
std::vector<Eigen::Vector3d> Animation::CSimulation::update()
{
	std::vector<Eigen::Vector3d> Positions;

	__updateBoneParticlePosition();

	// PBD update
	for (int i = 0; i < m_SimulationCountPerFrame; i++)
	{
		bool Result = m_pTimeStepController->step();
		if (!Result) return Positions;
	}

	Positions = m_pTimeStepController->getPositions();
	return Positions;
}

//*********************************************************************
//FUNCTION:
void Animation::CSimulation::__initParameters()
{
	m_IterationCount = CAnimationConfig::getInstance()->getAttribute<int>(Animation::KEY_WORDS::ITERATION_COUNT);
	m_DampFactor = CAnimationConfig::getInstance()->getAttribute<double>(Animation::KEY_WORDS::DAMP_FACTOR);
	m_TimeStep = CAnimationConfig::getInstance()->getAttribute<double>(Animation::KEY_WORDS::TIME_STEP_SIZE);

	const std::string GravityStr = CAnimationConfig::getInstance()->getAttribute<std::string>(KEY_WORDS::GRAVITY);
	double Gravity[3];
	hiveUtility::hiveString::hiveSplitLine2DoubleArray(GravityStr, ",", 3, Gravity);
	m_Gravity = Eigen::Vector3d(Gravity[0], Gravity[1], Gravity[2]);
}

//*********************************************************************
//FUNCTION:
void Animation::CSimulation::__initSimulationModel()
{
	if (m_pSimulationModel == nullptr)
	{
		m_pSimulationModel = boost::shared_ptr<CSimulationModel>(new CSimulationModel());
	}

	if (m_pParticleSource != nullptr)
	{
		// add particles
		const int ParticleCount = m_pParticleSource->getParticleCount();
		for (int i = 0; i < ParticleCount; i++)
		{
			Eigen::Vector3d ParticlePos = m_pParticleSource->getParticlePos(i);
;			m_pSimulationModel->addSimulationParticle(ParticlePos, m_Gravity);
		}
		std::cout << "particles_total_num:" << ParticleCount << std::endl;
		// set bone particles constant
		std::vector<SBoneParticle> BoneParticles;
		m_pParticleLayer->dumpBoneParticles(BoneParticles);
		for (int i = 0; i < BoneParticles.size(); i++)
		{
			const int Index = BoneParticles[i].ParticleIndex;
			m_pSimulationModel->setParticleMass(Index, 0.0);
		}

		std::ifstream CacheFile(m_CacheFilePath, std::fstream::in);
		if (CacheFile)
		{
			// generate constraints from CacheFile
			boost::archive::text_iarchive ia(CacheFile);
			ia >> *(m_pSimulationModel).get();
		}
		else
		{
			// generate constraints
			m_pSimulationModel->generateConstraint(m_pParticleSource, m_pParticleLayer, EConstraintsType::SHAPEMATCHING, 8);
			m_pSimulationModel->generateConstraint(m_pParticleSource, m_pParticleLayer, EConstraintsType::DISTANCE, 2);
			m_pSimulationModel->generateConstraint(m_pParticleSource, m_pParticleLayer, EConstraintsType::VOLUME, 4);
			std::ofstream OutCacheFile(m_CacheFilePath, std::fstream::out);
			boost::archive::text_oarchive oa(OutCacheFile);
			oa << *(m_pSimulationModel).get();
			OutCacheFile.close();
		}
		CacheFile.close();
	}
}

//*********************************************************************
//FUNCTION:
void Animation::CSimulation::__updateBoneParticlePosition()
{
	std::vector<SBoneParticle> BoneParticles;
	m_pParticleLayer->dumpBoneParticles(BoneParticles);
	std::vector<Eigen::Matrix4d> SkeletonTransfer;
	if (!m_pSkeletonController->dumpNextPose(SkeletonTransfer))
	{
		m_pSkeletonController->resetPose();
		m_pSimulationModel->resetSimulationState();
		m_pTimeStepController->resetParticleData(*m_pSimulationModel);
		return;
	}
	const int BoneParticleCount = BoneParticles.size();
	std::vector<int> Indices(BoneParticleCount);
	std::vector<SVector3d> Pos(BoneParticleCount);
	for (size_t i = 0; i < BoneParticleCount; i++)
	{
		Eigen::Vector3d WorldPos(0.0, 0.0, 0.0);

		for (int j = 0; j < INFLUENCE_NUM; j++)
		{
			SBindInfo BindInfo = BoneParticles[i].BindInfoList[j];
			int BoneIndex = BindInfo.BoneIndex;
			Eigen::Matrix4d LocalToWolrd = SkeletonTransfer[BoneIndex];
			Eigen::Vector3d LocalPos = BindInfo.LocalPosition;
			Eigen::Vector3d NewPos = translatePostion(LocalPos, LocalToWolrd);

			WorldPos += BindInfo.Weight * NewPos;
		}

		Indices[i] = BoneParticles[i].ParticleIndex;
		Pos[i] = SVector3d(WorldPos[0], WorldPos[1], WorldPos[2]);
	}

	m_pTimeStepController->updateBoneParticlesWithCuda(Indices, Pos);
}