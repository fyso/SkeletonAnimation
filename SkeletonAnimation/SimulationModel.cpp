#include "SimulationModel.h"
#include <fstream>
#include <iostream>
void Animation::CSimulationModel::generateConstraint(boost::shared_ptr<CParticleSource> vParticleSource, const boost::shared_ptr<CParticleLayer> vParticleLayer, const EConstraintsType vConstraintsType, int vParallelNumber)
{
	m_CountOfRealParticle = vParticleSource->getParticleCount();
	if (vConstraintsType == DISTANCE)
	{
		__generateDistanceConstraint(vParticleSource, vParticleLayer, vParallelNumber);
	}
	else if (vConstraintsType == SHAPEMATCHING)
	{
		__generateShapeMatchingConstraint(vParticleSource, vParticleLayer, vParallelNumber);
	}
	else if (vConstraintsType == VOLUME)
	{
		__generateVolumeConstraint(vParticleSource, vParticleLayer, vParallelNumber);
	}
}

void Animation::CSimulationModel::__generateShapeMatchingConstraint(const boost::shared_ptr<CParticleSource> vParticleSource, const boost::shared_ptr<CParticleLayer> vParticleLayer, int vParallelNumber)
{
	m_ParticleRelatedClusterCount.resize(m_CountOfRealParticle, 0);
	std::vector<std::vector<int>> TempIndexOfConstraint;

	const int RegionCount = vParticleSource->getRegionCount();
	for (int i = 0; i < RegionCount; i++)
	{
		SRegion Region = vParticleSource->getRegion(i);
		const int RegionSize = Region.RegionParticles.size();

		std::vector<double> Stiffness(RegionSize);
		int count = 0;
		for (int k = 0; k < RegionSize; k++)
		{
			const int Index = Region.RegionParticles[k];
			if (vParticleLayer->isBoneLayer(Index))
				count++;
			Stiffness[k] = vParticleLayer->getParticleStiffness(Index);
		}
		if (count == RegionSize)
			continue;

		CShapeMatchingConstraint Constraint(RegionSize);
		bool Result = Constraint.initializeConstraint(m_ParticleData, Region.RegionParticles, Region.NumOfRelatedRegionsInParticle, Stiffness);
		_ASSERTE(Result == true);
		TempIndexOfConstraint.push_back(Region.RegionParticles);
		m_ShapeMatchingConstraints.push_back(Constraint);

		for (int k = 0; k < RegionSize; k++)
		{
			int Index = Region.RegionParticles[k];
			m_ParticleRelatedClusterCount[Index] += 1;
		}
	}

	std::vector<std::vector<int>> index;
	if (vParallelNumber == 0)
	{
		std::vector<int> Temp;
		for (int i = 0; i < TempIndexOfConstraint.size(); i++)
		{
			Temp.push_back(i);
		}
		index.push_back(Temp);
	}
	else
	{
		index = __splitConstraintsWithLCF(TempIndexOfConstraint, vParallelNumber, SHAPEMATCHING);
	}
	
	std::vector<CShapeMatchingConstraint> ShapeMatchingConstraintGroup;
	for (int i = 0; i < index.size(); i++)
	{
		std::vector<CShapeMatchingConstraint> Temp;
		for (int k = 0; k < index[i].size(); k++)
		{
			Temp.push_back(m_ShapeMatchingConstraints[index[i][k]]);
		}

		ShapeMatchingConstraintGroup.insert(ShapeMatchingConstraintGroup.end(), Temp.begin(), Temp.end());
		m_ShapeMatchingConstraintsGroupOffset.push_back(ShapeMatchingConstraintGroup.size());
	}
	m_ShapeMatchingConstraints.assign(ShapeMatchingConstraintGroup.begin(), ShapeMatchingConstraintGroup.end());

	if (vParallelNumber > 0)
	{
		__generateShapeMatchingProjectionConstraints();
	}

	std::cout << "ShapeMatch_Constraint_Total_Num: " << m_ShapeMatchingConstraints.size() << " Group Num: " << index.size() << std::endl;
}

void Animation::CSimulationModel::__generateDistanceConstraint(const boost::shared_ptr<CParticleSource> vParticleSource, const boost::shared_ptr<CParticleLayer> vParticleLayer, int vParallelNumber)
{
	std::vector<std::vector<int>> TempIndexOfConstraint;

	CTetModel TetModel = vParticleSource->getTetModel();
	const int EdgeCount = TetModel.getEdgeSize();
	for (int i = 0; i < EdgeCount; i++)
	{
		std::pair<int, int> Edge = TetModel.getEdge(i);
		double Stiffness = (vParticleLayer->getParticleStiffness(Edge.first) + vParticleLayer->getParticleStiffness(Edge.second)) / 2.0;

		if (vParticleLayer->isBoneLayer(Edge.first) && vParticleLayer->isBoneLayer(Edge.second))
			continue;

		CDistanceConstraint Constraint;
		const bool Result = Constraint.initializeConstraint(m_ParticleData, std::pair<int, int>(Edge.first, Edge.second), Stiffness);
		_ASSERTE(Result == true);
		std::vector<int> temp;
		for (int k = 0; k < 2; k++)
		{
			temp.push_back(Constraint.getParticleIndices()[k]);
		}
		TempIndexOfConstraint.push_back(temp);
		m_DistanceConstraints.push_back(Constraint);
	}

	std::vector<std::vector<int>> index;
	if (vParallelNumber == 0)
	{
		std::vector<int> Temp;
		for (int i = 0; i < TempIndexOfConstraint.size(); i++) 
		{
			Temp.push_back(i);
		}
		index.push_back(Temp); 
	}
	else
	{
		index = __splitConstraintsWithLCF(TempIndexOfConstraint, vParallelNumber, DISTANCE);
	}

	std::vector<CDistanceConstraint> DistanceConstraintsGroup;
	for (int i = 0; i < index.size(); i++)
	{
		std::vector<CDistanceConstraint> Temp;
		for (int k = 0; k < index[i].size(); k++)
		{
			Temp.push_back(m_DistanceConstraints[index[i][k]]);
		}

		DistanceConstraintsGroup.insert(DistanceConstraintsGroup.end(), Temp.begin(), Temp.end());
		m_DistanceConstraintsGroupOffset.push_back(DistanceConstraintsGroup.size());
	}
	m_DistanceConstraints.assign(DistanceConstraintsGroup.begin(), DistanceConstraintsGroup.end());

	if (vParallelNumber > 0)
	{
		__generateDistanceProjectionConstraints();
	}

	std::cout << "Distance_Constraint_Total_Num: " << m_DistanceConstraints.size() << " Group Num: "<< index.size() << std::endl;
}

void Animation::CSimulationModel::__generateVolumeConstraint(const boost::shared_ptr<CParticleSource> vParticleSource, const boost::shared_ptr<CParticleLayer> vParticleLayer, int vParallelNumber)
{
	std::vector<std::vector<int>> TempIndexOfConstraint;

	CTetModel TetModel = vParticleSource->getTetModel();
	const int TetCount = TetModel.getTetSize();
	for (int i = 0; i < TetCount; i++)
	{
		std::vector<int> Tet = TetModel.getTet(i);
		double Stiffness = 0.0;
		int count = 0;
		for (auto ParticleIndex : Tet)
		{
			if (vParticleLayer->isBoneLayer(ParticleIndex))
				count++;
			Stiffness += vParticleLayer->getParticleStiffness(ParticleIndex);
		}
		if (count == Tet.size())
			continue;

		Stiffness /= static_cast<double>(Tet.size());

		CVolumeConstraint Constraint;
		const bool Result = Constraint.initializeConstraint(m_ParticleData, Tet, Stiffness);
		_ASSERTE(Result == true);
		TempIndexOfConstraint.push_back(Tet);
		m_VolumeConstraints.push_back(Constraint);
	}
	
	std::vector<std::vector<int>> index;
	if (vParallelNumber == 0)
	{
		std::vector<int> Temp;
		for (int i = 0; i < TempIndexOfConstraint.size(); i++)
		{
			Temp.push_back(i);
		}
		index.push_back(Temp);
	}
	else
	{
		index = __splitConstraintsWithLCF(TempIndexOfConstraint, vParallelNumber, VOLUME);
	}

	std::vector<CVolumeConstraint> VolumeConstraintsGroup;
	for (int i = 0; i < index.size(); i++)
	{
		std::vector<CVolumeConstraint> Temp;
		for (int k = 0; k < index[i].size(); k++)
		{
			Temp.push_back(m_VolumeConstraints[index[i][k]]);
		}

		VolumeConstraintsGroup.insert(VolumeConstraintsGroup.end(), Temp.begin(), Temp.end());
		m_VolumeConstraintsGroupOffset.push_back(VolumeConstraintsGroup.size());
	}
	m_VolumeConstraints.assign(VolumeConstraintsGroup.begin(), VolumeConstraintsGroup.end());

	if (vParallelNumber > 0)
	{
		__generateVolumeProjectionConstraints();
	}
	
	std::cout << "Volume_Constraint_Total_Num: " << m_VolumeConstraints.size() << " Group Num: " << index.size() << std::endl;
}

std::vector<std::vector<int>> Animation::CSimulationModel::__splitConstraintsWithLDF(const std::vector<std::vector<int>>& vSourceConstraints, int vParallelNumber, const EConstraintsType vConstraintsType)
{
	std::vector<int> BoneFlags(m_ParticleData.size(),0);
	for (int i = 0; i < m_ParticleData.size(); i++)
	{
		if (m_ParticleData[i].getMass() <= DBL_EPSILON)
			BoneFlags[i] = 1;
	}
	

	// generate CColorableGraph
	CColorableGraphPtr ColorGraph(new CColorableGraph(vSourceConstraints.size()));
	std::ifstream CacheFile("./ColorableGraphCache_" + std::to_string(vConstraintsType), std::fstream::in);
	if (CacheFile)
	{
		boost::archive::text_iarchive ia(CacheFile);
		ia >> *(ColorGraph).get();
		__generateColorableGraph(ColorGraph, m_ParticleData.size(), vSourceConstraints);
	}
	else
	{
		__generateColorableGraph(ColorGraph, m_ParticleData.size(), vSourceConstraints);
		ColorGraph->searchMaximalCliquesByBronKerbosch();

		std::ofstream OutCacheFile("./ColorableGraphCache_" + std::to_string(vConstraintsType), std::fstream::out);
		boost::archive::text_oarchive oa(OutCacheFile);
		oa << *(ColorGraph).get();
		OutCacheFile.close();
	}
	CacheFile.close();

	if (vParallelNumber > 0)
	{
		ColorGraph->setColorNumberQ(vParallelNumber);
	}
	
	std::vector<std::vector<int>> GroupResult = ColorGraph->colorGraphWithLDF();
	ColorGraph->writeColorGroupToFile(GroupResult, "../TestData/ColorGroup" + std::to_string(vConstraintsType) + ".txt");
	
	return GroupResult;
}

std::vector<std::vector<int>> Animation::CSimulationModel::__splitConstraintsWithLEF(const std::vector<std::vector<int>>& vSourceConstraints, int vParallelNumber, const EConstraintsType vConstraintsType)
{
	std::vector<int> BoneFlags(m_ParticleData.size(), 0);
	for (int i = 0; i < m_ParticleData.size(); i++)
	{
		if (m_ParticleData[i].getMass() <= DBL_EPSILON)
			BoneFlags[i] = 1;
	}


	// generate CColorableGraph
	CColorableGraphPtr ColorGraph(new CColorableGraph(vSourceConstraints.size()));
	std::ifstream CacheFile("./ColorableGraphCache_" + std::to_string(vConstraintsType), std::fstream::in);
	if (CacheFile)
	{
		boost::archive::text_iarchive ia(CacheFile);
		ia >> *(ColorGraph).get();
		__generateColorableGraph(ColorGraph, m_ParticleData.size(), vSourceConstraints);
	}
	else
	{
		__generateColorableGraph(ColorGraph, m_ParticleData.size(), vSourceConstraints);
		ColorGraph->searchMaximalCliquesByBronKerbosch();

		std::ofstream OutCacheFile("./ColorableGraphCache_" + std::to_string(vConstraintsType), std::fstream::out);
		boost::archive::text_oarchive oa(OutCacheFile);
		oa << *(ColorGraph).get();
		OutCacheFile.close();
	}
	CacheFile.close();

	if (vParallelNumber > 0)
	{
		ColorGraph->setColorNumberQ(vParallelNumber);
	}

	std::vector<std::vector<int>> GroupResult = ColorGraph->colorGraphWithLEF();
	ColorGraph->writeColorGroupToFile(GroupResult, "../TestData/ColorGroup" + std::to_string(vConstraintsType) + ".txt");

	return GroupResult;
}

std::vector<std::vector<int>> Animation::CSimulationModel::__splitConstraintsWithLCF(const std::vector<std::vector<int>>& vSourceConstraints, int vParallelNumber, const EConstraintsType vConstraintsType)
{
	std::vector<int> BoneFlags(m_ParticleData.size(), 0);
	for (int i = 0; i < m_ParticleData.size(); i++)
	{
		if (m_ParticleData[i].getMass() <= DBL_EPSILON)
			BoneFlags[i] = 1;
	}


	// generate CColorableGraph
	CColorableGraphPtr ColorGraph(new CColorableGraph(vSourceConstraints.size()));
	std::ifstream CacheFile("./ColorableGraphCache_" + std::to_string(vConstraintsType), std::fstream::in);
	if (CacheFile)
	{
		boost::archive::text_iarchive ia(CacheFile);
		ia >> *(ColorGraph).get();
		__generateColorableGraph(ColorGraph, m_ParticleData.size(), vSourceConstraints);
	}
	else
	{
		__generateColorableGraph(ColorGraph, m_ParticleData.size(), vSourceConstraints);
		ColorGraph->searchMaximalCliquesByBronKerbosch();

		std::ofstream OutCacheFile("./ColorableGraphCache_" + std::to_string(vConstraintsType), std::fstream::out);
		boost::archive::text_oarchive oa(OutCacheFile);
		oa << *(ColorGraph).get();
		OutCacheFile.close();
	}
	CacheFile.close();

	if (vParallelNumber > 0)
	{
		ColorGraph->setColorNumberQ(vParallelNumber);
	}

	std::vector<std::vector<int>> GroupResult = ColorGraph->colorGraphWithClique();
	ColorGraph->writeColorGroupToFile(GroupResult, "../TestData/ColorGroup" + std::to_string(vConstraintsType) + ".txt");

	return GroupResult;
}

void Animation::CSimulationModel::__generateColorableGraph(CColorableGraphPtr voColorGraph, int vParticlesCount, const std::vector<std::vector<int>>& vTargetConstraints)
{
	std::vector<std::vector<int>> ParticlesConstraintsTable;
	ParticlesConstraintsTable.resize(vParticlesCount);

	for (int i = 0; i < vTargetConstraints.size(); i++)
	{
		std::vector<int> ParticleSet = vTargetConstraints[i];
		for (int k = 0; k < ParticleSet.size(); k++)
		{
			int currentParticleIndex = ParticleSet[k];
			//Ìø¹ý¹Ç÷ÀÁ£×Ó
			if (m_ParticleData[currentParticleIndex].getMass() <= DBL_EPSILON)
				continue;

			std::vector<int> ConstraintsInCurrentParticle = ParticlesConstraintsTable[currentParticleIndex];
			if (ConstraintsInCurrentParticle.size() != 0)
			{
				for (int j = 0; j < ConstraintsInCurrentParticle.size(); j++)
				{
					voColorGraph->addEdgeV(ConstraintsInCurrentParticle[j], i);
				}
			}

			ParticlesConstraintsTable[currentParticleIndex].push_back(i);
		}
	}
}

void Animation::CSimulationModel::__deleteEdgesInClique(const std::vector<int>& vInputClique, int vColorNumberQ, EConstraintsType vConstraintType, CColorableGraphPtr voColorGraph)
{
	if (vInputClique.size() <= vColorNumberQ) return;

	std::map<int, std::vector<int>> ParticlesConstraionMap;
	for (auto ConstraintIndex : vInputClique)
	{
		switch (vConstraintType)
		{
		case DISTANCE:
			for (int i = 0; i < 2; i++)
			{
				int TargetParticleIndex = m_DistanceConstraints[ConstraintIndex].getParticleIndices()[i];

				if (m_ParticleData[TargetParticleIndex].getMass() <= DBL_EPSILON)
					continue;
				if (ParticlesConstraionMap.count(TargetParticleIndex) == 0)
				{
					ParticlesConstraionMap[TargetParticleIndex] = std::vector<int>();
					ParticlesConstraionMap[TargetParticleIndex].push_back(ConstraintIndex);
				}
				else
				{
					ParticlesConstraionMap[TargetParticleIndex].push_back(ConstraintIndex);
				}
			}
			break;
		case VOLUME:
			for (int i = 0; i < 4; i++)
			{
				int TargetParticleIndex = m_VolumeConstraints[ConstraintIndex].getParticleIndices()[i];

				if (m_ParticleData[TargetParticleIndex].getMass() <= DBL_EPSILON)
					continue;

				if (ParticlesConstraionMap.count(TargetParticleIndex) == 0)
				{
					ParticlesConstraionMap[TargetParticleIndex] = std::vector<int>();
					ParticlesConstraionMap[TargetParticleIndex].push_back(ConstraintIndex);
				}
				else
				{
					ParticlesConstraionMap[TargetParticleIndex].push_back(ConstraintIndex);
				}
			}
			break;
		case SHAPEMATCHING:
			for (int i = 0; i < m_ShapeMatchingConstraints[ConstraintIndex].getParticleCount(); i++)
			{
				int TargetParticleIndex = m_ShapeMatchingConstraints[ConstraintIndex].getParticleIndices()[i];

				if (m_ParticleData[TargetParticleIndex].getMass() <= DBL_EPSILON)
					continue;

				if (ParticlesConstraionMap.count(TargetParticleIndex) == 0)
				{
					ParticlesConstraionMap[TargetParticleIndex] = std::vector<int>();
					ParticlesConstraionMap[TargetParticleIndex].push_back(ConstraintIndex);
				}
				else
				{
					ParticlesConstraionMap[TargetParticleIndex].push_back(ConstraintIndex);
				}
			}
			break;
		}
	}
	
	std::vector<std::pair<int, std::vector<int>>> VecPair(ParticlesConstraionMap.begin(), ParticlesConstraionMap.end());
	sort(VecPair.begin(), VecPair.end(), [&](const std::pair<int, std::vector<int>>& vLeft, const std::pair<int, std::vector<int>>& vRight) {return vLeft.second.size() > vRight.second.size(); });
	int count = 0;
	while (voColorGraph->searchCliqueInChildGraph(vInputClique) && count < VecPair.size())
	{		
		int MaxConstrintNumParticleIndex = VecPair[count].first;
		if (MaxConstrintNumParticleIndex >= m_CountOfRealParticle)
		{
			std::cout << MaxConstrintNumParticleIndex << ": " << vInputClique.size() << " ," << ParticlesConstraionMap[MaxConstrintNumParticleIndex].size() << std::endl;
		}
		if (vConstraintType == DISTANCE)
		{
			__generateDistanceDuplicateParticles(vInputClique, MaxConstrintNumParticleIndex);
		}
		else if (vConstraintType == VOLUME)
		{
			__generateVolumeDuplicateParticles(vInputClique, MaxConstrintNumParticleIndex);
		}
		else if (vConstraintType == SHAPEMATCHING)
		{
			__generateShapeMatchingDuplicateParticles(vInputClique, MaxConstrintNumParticleIndex);
		}

		std::vector<int> CheckConstraints = ParticlesConstraionMap[MaxConstrintNumParticleIndex];
		ParticlesConstraionMap.erase(MaxConstrintNumParticleIndex);
		for (int i = 0; i < CheckConstraints.size() - 1; i++)
		{
			for (int t = i + 1; t < CheckConstraints.size(); t++)
			{
				if (!__ifTwoConstraintConnected(ParticlesConstraionMap, CheckConstraints[i], CheckConstraints[t]))
				{
					voColorGraph->deleteEdgeV(CheckConstraints[i], CheckConstraints[t]);
				}
			}
		}
		count++;
	}	
}

bool Animation::CSimulationModel::__ifTwoConstraintConnected(const std::map<int, std::vector<int>>& vParticlesConstraionMap, int vConstraintIdx0, int vConstraintIdx1)
{
	for (auto Pair : vParticlesConstraionMap)
	{
		bool flag0 = false, flag1 = false;
		for (int i = 0; i < Pair.second.size(); i++)
		{
			if (Pair.second[i] == vConstraintIdx0)
				flag0 = true;
			if (Pair.second[i] == vConstraintIdx1)
				flag1 = true;
		}
		if (flag0 && flag1)
			return true;
	}
	return false;
}

//*********************************************************************
//FUNCTION:
void Animation::CSimulationModel::addSimulationParticle(const Eigen::Vector3d& vPos, const Eigen::Vector3d& vGravity)
{
	SVector3d Position(vPos[0], vPos[1], vPos[2]);
	SVector3d Gravity(vGravity[0], vGravity[1], vGravity[2]);
	CParticleData ParticleData;
	ParticleData.setInfo(Position, Gravity);

	m_ParticleData.push_back(ParticleData);
}

//*********************************************************************
//FUNCTION:
void Animation::CSimulationModel::resetSimulationState()
{
	for (int i = 0; i < m_ParticleData.size(); i++)
	{
		m_ParticleData[i].resetState();
	}
}

//*********************************************************************
//FUNCTION:
void Animation::CSimulationModel::dumpSimulationParticle(std::vector<CParticleData>& voConstraints) const
{
	voConstraints = m_ParticleData;
}

//*********************************************************************
//FUNCTION:
void Animation::CSimulationModel::dumpDistanceConstraints(std::vector<CDistanceConstraint>& voConstraints) const
{
	voConstraints = m_DistanceConstraints;
}

//*********************************************************************
//FUNCTION:
void Animation::CSimulationModel::dumpDistanceProjectionConstraints(std::vector<CProjectionConstraint>& voConstraints) const
{
	voConstraints = m_DistanceProjectionConstraints;
}

//*********************************************************************
//FUNCTION:
void Animation::CSimulationModel::dumpVolumeConstraints(std::vector<CVolumeConstraint>& voConstraints) const
{
	voConstraints = m_VolumeConstraints;
}

//*********************************************************************
//FUNCTION:
void Animation::CSimulationModel::dumpVolumeProjectionConstraints(std::vector<CProjectionConstraint>& voConstraints) const
{
	voConstraints = m_VolumeProjectionConstraints;
}

//*********************************************************************
//FUNCTION:
void Animation::CSimulationModel::dumpShapeMatchingConstraints(std::vector<CShapeMatchingConstraint>& voConstraints) const
{
	voConstraints = m_ShapeMatchingConstraints;
}

//*********************************************************************
//FUNCTION:
void Animation::CSimulationModel::dumpShapeMatchingProjectionConstraints(std::vector<CProjectionConstraint>& voConstraints) const
{
	voConstraints = m_ShapeMatchingProjectionConstraints;
}

//*********************************************************************
//FUNCTION:
void Animation::CSimulationModel::setParticleMass(int vIndex, double vMass)
{
	_ASSERTE(vIndex >= 0 && vIndex < m_ParticleData.size());
	_ASSERTE(!(vMass < 0.0));

	m_ParticleData[vIndex].setMass(vMass);
}

int Animation::CSimulationModel::getVolumeDuplicateparticleNumber() const
{
	int sum = 0;
	for (auto MainParticle : m_VolumeDuplicateParticles)
	{
		sum += MainParticle.second.size();
	}
	return sum;
}

int Animation::CSimulationModel::getShapeMatchingDuplicateparticleNumber() const
{
	int sum = 0;
	for (auto MainParticle : m_ShapeMatchingDuplicateParticles)
	{
		sum += MainParticle.second.size();
	}
	return sum;
}

int Animation::CSimulationModel::getDistanceDuplicateparticleNumber() const
{
	int sum = 0;
	for (auto MainParticle : m_DistanceDuplicateParticles)
	{
		sum += MainParticle.second.size();
	}
	return sum;
}

void Animation::CSimulationModel::__generateDistanceDuplicateParticles(const std::vector<int>& vInputClique, int vTargetParticleIndex)
{
	if (m_ParticleData[vTargetParticleIndex].getMass() <= DBL_EPSILON)
	{
		return;
	}

	for (auto ConstraintIndex : vInputClique)
	{
		int* ParticleList = m_DistanceConstraints[ConstraintIndex].getParticleIndices();
		for (int k = 0; k < 2; k++)
		{
			if (ParticleList[k] == vTargetParticleIndex)
			{
				if (m_DistanceDuplicateParticles.count(vTargetParticleIndex) == 0)
				{
					std::vector<int> temp;
					temp.push_back(vTargetParticleIndex);
					m_DistanceDuplicateParticles[vTargetParticleIndex] = temp;
				}
				else
				{
					CParticleData ParticleData = m_ParticleData[vTargetParticleIndex];
					m_ParticleData.push_back(ParticleData);
					ParticleList[k] = m_ParticleData.size() - 1;
					m_DistanceDuplicateParticles[vTargetParticleIndex].push_back(ParticleList[k]);
				}
				break;
			}
		}
	}
}

void Animation::CSimulationModel::__generateVolumeDuplicateParticles(const std::vector<int>& vInputClique, int vTargetParticleIndex)
{
	if (m_ParticleData[vTargetParticleIndex].getMass() <= DBL_EPSILON)
	{
		return;
	}
	for (auto ConstraintIndex : vInputClique)
	{
		int* ParticleList = m_VolumeConstraints[ConstraintIndex].getParticleIndices();
		for (int k = 0; k < 4; k++)
		{
			if (ParticleList[k] == vTargetParticleIndex)
			{
				if (m_VolumeDuplicateParticles.count(vTargetParticleIndex) == 0)
				{
					std::vector<int> temp;
					temp.push_back(vTargetParticleIndex);
					m_VolumeDuplicateParticles[vTargetParticleIndex] = temp;
				}
				else
				{
					CParticleData ParticleData = m_ParticleData[vTargetParticleIndex];
					m_ParticleData.push_back(ParticleData);
					ParticleList[k] = m_ParticleData.size() - 1;
					m_VolumeDuplicateParticles[vTargetParticleIndex].push_back(ParticleList[k]);
				}
				break;
			}
		}
	}
}

void Animation::CSimulationModel::__generateShapeMatchingDuplicateParticles(const std::vector<int>& vInputClique, int vTargetParticleIndex)
{
	if (m_ParticleData[vTargetParticleIndex].getMass() <= DBL_EPSILON)
	{
		return;
	}
	for (auto ConstraintIndex : vInputClique)
	{
		int* ParticleList = m_ShapeMatchingConstraints[ConstraintIndex].getParticleIndices();
		int count = m_ShapeMatchingConstraints[ConstraintIndex].getParticleCount();
		for (int k = 0; k < count; k++)
		{
			if (ParticleList[k] == vTargetParticleIndex)
			{
				if (m_ShapeMatchingDuplicateParticles.count(vTargetParticleIndex) == 0)
				{
					std::vector<int> temp;
					temp.push_back(vTargetParticleIndex);
					m_ShapeMatchingDuplicateParticles[vTargetParticleIndex] = temp;
				}
				else
				{
					CParticleData ParticleData = m_ParticleData[vTargetParticleIndex];
					m_ParticleData.push_back(ParticleData);
					ParticleList[k] = m_ParticleData.size() - 1;
					m_ShapeMatchingDuplicateParticles[vTargetParticleIndex].push_back(ParticleList[k]);
				}
				break;
			}
		}
	}
}

void Animation::CSimulationModel::__writeGraphToFile(const std::vector<std::vector<int>>& vGraph, const std::string& vCacheFileName)
{
	std::ofstream FileOut(vCacheFileName, std::fstream::out);
	if (FileOut)
	{
		for (int i = 0; i < vGraph.size(); i++)
		{
			FileOut << i << " ";
			for (int Node : vGraph[i])
			{
				FileOut << Node << " ";
			}
			FileOut << ";" << std::endl;
		}
	}
	FileOut.close();
}

void Animation::CSimulationModel::__generateDistanceProjectionConstraints()
{
	for (auto elem : m_DistanceDuplicateParticles)
	{
		if (elem.second.size() > 1)
		{
			CProjectionConstraint Constraint;
			Constraint.initializeConstraint(elem.second);
			m_DistanceProjectionConstraints.push_back(Constraint);
		}
	}
	std::cout << "distance projection constraints total num:" << m_DistanceProjectionConstraints.size() << std::endl;
}

void Animation::CSimulationModel::__generateVolumeProjectionConstraints()
{
	for (auto elem : m_VolumeDuplicateParticles)
	{
		if (elem.second.size() > 1)
		{
			CProjectionConstraint Constraint;
			Constraint.initializeConstraint(elem.second);
			m_VolumeProjectionConstraints.push_back(Constraint);
		}
	}
	std::cout << "volume projection constraints total num:" << m_VolumeProjectionConstraints.size() << std::endl;
}

void Animation::CSimulationModel::__generateShapeMatchingProjectionConstraints()
{
	for (auto elem : m_ShapeMatchingDuplicateParticles)
	{
		if (elem.second.size() > 1)
		{
			CProjectionConstraint Constraint;
			Constraint.initializeConstraint(elem.second);
			m_ShapeMatchingProjectionConstraints.push_back(Constraint);
			m_ParticleRelatedClusterCount[elem.first] = m_ParticleRelatedClusterCount[elem.first] - elem.second.size() + 1;
			_ASSERTE(m_ParticleRelatedClusterCount[elem.first] >= 1);
		}
	}
	std::vector<int> duplicateParticles(m_ParticleData.size() - m_CountOfRealParticle, 1);
	m_ParticleRelatedClusterCount.insert(m_ParticleRelatedClusterCount.end(), duplicateParticles.begin(), duplicateParticles.end());
	std::cout << "ShapeMatch projection constraints total num:" << m_ShapeMatchingProjectionConstraints.size() << std::endl;
}
