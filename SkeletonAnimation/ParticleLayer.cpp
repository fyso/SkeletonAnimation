#include "ParticleLayer.h"
#include "AnimationConfig.h"

Animation::CParticleLayer::~CParticleLayer()
{
	__clear();
}

//*********************************************************************
//FUNCTION:
void Animation::CParticleLayer::initialize(CParticleSource* vParticleSource, CSkeletonController* vSkeletonController)
{
	_ASSERTE(vParticleSource != nullptr && vSkeletonController != nullptr);
	m_pParticleSource = vParticleSource;
	m_pSkeletonController = vSkeletonController;

	const int ParticleCount = vParticleSource->getParticleCount();
	m_ParticleStiffness.resize(ParticleCount);
	m_ParticleColor.resize(ParticleCount);

	const std::string StiffnessStr = CAnimationConfig::getInstance()->getAttribute<std::string>(KEY_WORDS::STIFFNESS_VALUE);
	double StiffnessValue[4];
	hiveUtility::hiveString::hiveSplitLine2DoubleArray(StiffnessStr, ",", 4, StiffnessValue);
	m_StiffnessValue = std::vector<double>(StiffnessValue, StiffnessValue + sizeof(StiffnessValue) / sizeof(double));
	m_SmoothParameter = CAnimationConfig::getInstance()->getAttribute<double>(KEY_WORDS::SMOOTH_PARAMETER);

	__initGraph();
	__initBoneLayer();
	__generateBindInfo();
	__generateSkinLayer();
	__generateMuscleAndFatLayer();
}

//*********************************************************************
//FUNCTION:
double Animation::CParticleLayer::getParticleStiffness(int vIndex) const
{
	_ASSERTE(vIndex >= 0 && vIndex < m_ParticleStiffness.size());
	return m_ParticleStiffness[vIndex];
}

//*********************************************************************
//FUNCTION:
Eigen::Vector3f Animation::CParticleLayer::getParticleColor(int vIndex) const
{
	_ASSERTE(vIndex >= 0 && vIndex < m_ParticleColor.size());
	return m_ParticleColor[vIndex];
}

//*********************************************************************
//FUNCTION:
bool Animation::CParticleLayer::isBoneLayer(int vIndex) const
{
	auto itr = find(m_BoneLayer.begin(), m_BoneLayer.end(), vIndex);
	if (itr != m_BoneLayer.end())
		return true;
	return false;
}

//*********************************************************************
//FUNCTION:
void Animation::CParticleLayer::__clear()
{
	if (m_pGraph)
		delete m_pGraph;
	m_pGraph = nullptr;

	m_pParticleSource = nullptr;
	m_pSkeletonController = nullptr;

	m_BoneSegments.clear();
	m_BoneLayer.clear();
	m_SkinLayer.clear();
	m_ParticleStiffness.clear();
	m_ParticleColor.clear();
}

//*********************************************************************
//FUNCTION:
void Animation::CParticleLayer::__initGraph()
{
	m_pGraph = new CUndirectedGraph(m_pParticleSource->getParticleCount());
	std::vector<int> CommonNeighbor;
	for (int i = 0; i < m_pParticleSource->getParticleCount(); i++)
	{
		CommonNeighbor.clear();
		m_pParticleSource->dumpCommonNeighbors(i, CommonNeighbor);
		for (int k = 0; k < CommonNeighbor.size(); k++)
		{
			m_pGraph->addEdge(i, CommonNeighbor[k]);
		}
	}
}

//*********************************************************************
//FUNCTION:
void Animation::CParticleLayer::__initBoneLayer()
{
	_ASSERTE(m_pParticleSource != nullptr && m_pSkeletonController != nullptr);

	m_BoneSegments.resize(m_pSkeletonController->getBoneCount() - 1);

	const int ParticleCount = m_pParticleSource->getParticleCount();
	const int BoneCount = m_pSkeletonController->getBoneCount();
#pragma omp parallel for
	for (int i = 1; i < BoneCount; i++)
	{
		SBoneSegment BoneSegment(m_pSkeletonController->getParentIndexAt(i), i);
		for (int k = 0; k < ParticleCount; k++)
		{
			Eigen::Vector3d ParticlePos = m_pParticleSource->getParticlePos(k);
			Eigen::Vector3d StartPos = m_pSkeletonController->getInitialBoneWorldPos(BoneSegment.StartBoneIndex);
			Eigen::Vector3d EndPos = m_pSkeletonController->getInitialBoneWorldPos(BoneSegment.EndBoneIndex);
			double Radius = m_pParticleSource->getParticleRadius();

			if (__isIntersect(StartPos, EndPos, ParticlePos, Radius))
			{
				BoneSegment.IntersetParticles.push_back(k);

				SBoneParticle BoneParticle(k);
				auto Itr = find(m_BoneLayer.begin(), m_BoneLayer.end(), BoneParticle);
				if (Itr == m_BoneLayer.end())
				{
#pragma omp critical
					{
						m_BoneLayer.push_back(BoneParticle);
					}
					m_ParticleStiffness[BoneParticle.ParticleIndex] = m_StiffnessValue[ParticleType::BONE];
					m_ParticleColor[BoneParticle.ParticleIndex] = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
					m_pGraph->generateShortestDistanceOf(k);
				}
			}
		}
		_ASSERTE(BoneSegment.IntersetParticles.size() > 0);
		m_BoneSegments[i - 1] = BoneSegment;
	}
}

//*********************************************************************
//FUNCTION:
void Animation::CParticleLayer::__generateBindInfo()
{
	const int BoneLayerCount = m_BoneLayer.size();
	//TODO: #pragma omp parallel for
	for (int i = 0; i < BoneLayerCount; i++)
	{
		int ParticleIndex = m_BoneLayer[i].ParticleIndex;
		const int BoneSegementCount = m_BoneSegments.size();
		std::vector<int> ShortestDistanceToBone(BoneSegementCount);

#pragma omp parallel for
		for (int k = 0; k < BoneSegementCount; k++)
		{
			std::vector<int> ShortestDistance(m_BoneSegments[k].IntersetParticles.size());
			for (int j = 0; j < m_BoneSegments[k].IntersetParticles.size(); j++)
			{
				const int Index = m_BoneSegments[k].IntersetParticles[j];
				ShortestDistance[j] = m_pGraph->findShortestDistance(Index, ParticleIndex);
			}

			int MinDistance = *std::min_element(ShortestDistance.begin(), ShortestDistance.end());
			MinDistance = MinDistance > 0 ? MinDistance : 1;//最短距离可能是自己到自己
			ShortestDistanceToBone[k] = MinDistance;
		}

		std::vector<int> NearestBoneSegmentIndex(INFLUENCE_NUM);
		__sortDistance(ShortestDistanceToBone, NearestBoneSegmentIndex);

		Eigen::Vector3i BoxExtend = m_pParticleSource->getBoxExtendSize();
		float BoxExtends = BoxExtend[0] + BoxExtend[1] + BoxExtend[2];
		float TotalWeight = 0.0f;
		for (int k = 0; k < INFLUENCE_NUM; k++)
		{
			int BoneSegmentIndex = NearestBoneSegmentIndex[k];
			int ParentBoneIndex = m_BoneSegments[BoneSegmentIndex].StartBoneIndex;

			m_BoneLayer[i].BindInfoList[k].BoneIndex = ParentBoneIndex;

			m_BoneLayer[i].BindInfoList[k].Weight = __calculateBindWeight(BoxExtends, ShortestDistanceToBone[BoneSegmentIndex]);

			Eigen::Matrix4d worldToLocalTransfer = m_pSkeletonController->getWorldToLocalTransfer(ParentBoneIndex);
			Eigen::Vector3d worldPos = m_pParticleSource->getParticlePos(ParticleIndex);
			m_BoneLayer[i].BindInfoList[k].LocalPosition = translatePostion(worldPos, worldToLocalTransfer);

			TotalWeight += m_BoneLayer[i].BindInfoList[k].Weight;
		}

		for (int k = 0; k < INFLUENCE_NUM; k++)
		{
			m_BoneLayer[i].BindInfoList[k].Weight /= TotalWeight;
		}
	}
}

//*********************************************************************
//FUNCTION:
void Animation::CParticleLayer::__generateSkinLayer()
{
	int SkinParticleCount = m_pParticleSource->getSkinParticleCount();
	std::vector<int> SkinParticles;
	m_pParticleSource->dumpSkinParticleIndices(SkinParticles);

	std::vector<int> AllParticles;
	for (int i = 0; i < m_pParticleSource->getParticleCount(); i++)
	{
		AllParticles.push_back(i);
	}

	std::vector<int> BoneParticleIndices;
	for (auto BoneParticle : m_BoneLayer)
		BoneParticleIndices.push_back(BoneParticle.ParticleIndex);

	m_SkinLayer.resize(SkinParticleCount);
#pragma omp parallel for
	for (int i = 0; i < SkinParticleCount; i++)
	{
		SSkinParticle SkinParticle(SkinParticles[i]);
		std::vector<int> DistanceToSkin = m_pGraph->findShortestDistanceByDijskra(SkinParticles[i], AllParticles);
		std::vector<int> DistanceToBone = m_pGraph->findShortestDistanceByDijskra(SkinParticles[i], BoneParticleIndices);

		int maxDis = *std::max_element(DistanceToSkin.begin(), DistanceToSkin.end());
		_ASSERTE(maxDis < MAX_DISTANCE);
		_ASSERTE(maxDis > 0);

		SkinParticle.MaxDistanceToSkin = maxDis;
		m_SkinLayer[i] = SkinParticle;
		m_ParticleStiffness[SkinParticle.ParticleIndex] = m_StiffnessValue[ParticleType::SKIN];
		m_ParticleColor[SkinParticle.ParticleIndex] = Eigen::Vector3f(0.85f, 0.57f, 0.57f);
	}
}

//*********************************************************************
//FUNCTION:
void Animation::CParticleLayer::__generateMuscleAndFatLayer()
{
	const double MuscleFatRatio = CAnimationConfig::getInstance()->getAttribute<double>(KEY_WORDS::MUSCLE_FAT_RATIO);

	std::vector<int> BoneParticleIndices, SkinParticleIndices;
	for (auto BoneParticle : m_BoneLayer)
		BoneParticleIndices.push_back(BoneParticle.ParticleIndex);
	for (auto SkinParticle : m_SkinLayer)
		SkinParticleIndices.push_back(SkinParticle.ParticleIndex);

	const int ParticleCount = m_pParticleSource->getParticleCount();
#pragma omp parallel for
	for (int i = 0; i < ParticleCount; i++)
	{
		if (m_ParticleStiffness[i] < FLT_EPSILON)
		{
			std::vector<int> DistanceToBone = m_pGraph->findShortestDistanceByDijskra(i, BoneParticleIndices);
			std::vector<int> DistanceToSkin = m_pGraph->findShortestDistanceByDijskra(i, SkinParticleIndices);

			int MinDistanceBoneIndex = -1;
			float MinDistanceToBone = FLT_MAX;
			for (int k = 0; k < DistanceToBone.size(); k++)
			{
				if (MinDistanceToBone > DistanceToBone[k])
				{
					MinDistanceToBone = DistanceToBone[k];
					MinDistanceBoneIndex = BoneParticleIndices[k];
				}
			}
			float MinDistanceToSkin = static_cast<float>(*(std::min_element(DistanceToSkin.begin(), DistanceToSkin.end())));

			if (MinDistanceToBone / MinDistanceToSkin <= MuscleFatRatio)
			{
				m_ParticleStiffness[i] = m_StiffnessValue[ParticleType::MUSCLE];
				m_ParticleColor[i] = Eigen::Vector3f(1.0f, 0.0f, 0.0f);
			}
			else
			{
				m_ParticleStiffness[i] = m_StiffnessValue[ParticleType::FAT];
				m_ParticleColor[i] = Eigen::Vector3f(0.0f, 1.0f, 0.0f);
			}
		}
	}
}

//*********************************************************************
//FUNCTION:
void Animation::CParticleLayer::__sortDistance(std::vector<int> vSource, std::vector<int>& voResult)
{
	const int SourceCount = vSource.size();
	const int ResultCount = voResult.size();
	_ASSERTE(SourceCount >= ResultCount);
	std::vector<int> ChangeIndex(vSource.size());
	for (int i = 0; i < vSource.size(); i++)
	{
		ChangeIndex[i] = i;
	}

	for (int i = 0; i < ResultCount; i++)
	{
		for (int j = i + 1; j < SourceCount; j++)
		{
			if (vSource[j] < vSource[i])
			{
				std::swap(vSource[j], vSource[i]);
				std::swap(ChangeIndex[j], ChangeIndex[i]);
			}
		}
		voResult[i] = ChangeIndex[i];
	}
}

//*********************************************************************
//FUNCTION:
float Animation::CParticleLayer::__calculateBindWeight(float vBoxExtend, float vDistance)
{
	float NormalizedDistance = vDistance / vBoxExtend;
	float Denominator = (1 - m_SmoothParameter) * NormalizedDistance + m_SmoothParameter * NormalizedDistance * NormalizedDistance;
	return pow(1 / Denominator, 2);
}

//*********************************************************************
//FUNCTION:
bool Animation::CParticleLayer::__isIntersect(Eigen::Vector3d vBoneStartPos, Eigen::Vector3d vBoneEndPos, Eigen::Vector3d vParticlePos, double vParticleRadius)
{
	Eigen::Vector3d MaxCorner = vParticlePos + Eigen::Vector3d(vParticleRadius, vParticleRadius, vParticleRadius);
	Eigen::Vector3d MinCorner = vParticlePos - Eigen::Vector3d(vParticleRadius, vParticleRadius, vParticleRadius);
	if (__isInVoxel(vBoneStartPos, MaxCorner, MinCorner) || __isInVoxel(vBoneEndPos, MaxCorner, MinCorner))
		return true;

	Eigen::Vector3d BoneVector = vBoneEndPos - vBoneStartPos;
	Eigen::Vector3d ParticleOffset = vParticlePos - vBoneStartPos;
	float DotResult = (ParticleOffset.normalized()).dot(BoneVector.normalized());
	DotResult = DotResult * ParticleOffset.norm();

	if (DotResult >= 0.0f && DotResult <= BoneVector.norm())
	{
		Eigen::Vector3d MapPos = DotResult * BoneVector.normalized() + vBoneStartPos;
		return __isInVoxel(MapPos, MaxCorner, MinCorner);
	}

	return false;
}

//*********************************************************************
//FUNCTION:
bool Animation::CParticleLayer::__isInVoxel(const Eigen::Vector3d& vPos, const Eigen::Vector3d& vMaxCorner, const Eigen::Vector3d& vMinCorner) const
{
	if ((vPos.x() < vMaxCorner.x()) && (vPos.x() > vMinCorner.x()) && (vPos.y() < vMaxCorner.y()) && (vPos.y() > vMinCorner.y()) && (vPos.z() < vMaxCorner.z()) && (vPos.z() > vMinCorner.z()))
		return true;

	return false;
}
