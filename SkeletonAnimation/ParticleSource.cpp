#include "ParticleSource.h"
#include <iostream>

//*********************************************************************
//FUNCTION:
void Animation::CParticleSource::initialize()
{
	_ASSERTE(m_Voxels.size() > 0);
	__createParticlesFromVoxels();

	__generateOneRingNeighbors();
	__generateCommonNeighbors();
	__generateRegions();
}

//*********************************************************************
//FUNCTION:
void Animation::CParticleSource::setVoxelizationInfo(const SVoxelizationInfo& vVoxelizationInfo)
{
	_ASSERTE(vVoxelizationInfo.Size > 0);

	m_VoxelInfo = vVoxelizationInfo;
}

//*********************************************************************
//FUNCTION:
void Animation::CParticleSource::addVoxel(int vSplitingCount, Eigen::Vector3i vLocation, Eigen::Vector3d vVoxelPos)
{
	SVoxelInfo Voxel(vVoxelPos);
	Voxel.SplitingCount = vSplitingCount;
	m_Voxels.insert(std::make_pair(vLocation, Voxel));
}

//*********************************************************************
//FUNCTION:
std::vector<int> Animation::CParticleSource::findNearestVertex(const Eigen::Vector3d& vPosition)
{
	Eigen::Vector3i Location;
	Eigen::Vector3d StartPosition = m_VoxelInfo.StartPos;
	for (int i = 0; i < 3; i++)
		Location[i] = static_cast<int>((vPosition[i] - StartPosition[i]) / m_VoxelInfo.Size + 0.5);

	std::vector<int> RelatedParticles;
	try
	{
		SVoxelInfo VoxelInfo = m_Voxels.at(Location);
		RelatedParticles = VoxelInfo.RelatedParticles;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl << std::endl;
	}

	return RelatedParticles;
}

//*********************************************************************
//FUNCTION:
void Animation::CParticleSource::dumpCommonNeighbors(int vParticleIndex, std::vector<int>& voNeighbors) const
{
	_ASSERTE(vParticleIndex >= 0 && vParticleIndex < m_ParticleNeighborList.size());
	const auto NeighborsSet = m_ParticleNeighborList[vParticleIndex].CommonNeighbors;

	voNeighbors.reserve(NeighborsSet.size());
	for (auto NeighborIndex : NeighborsSet)
	{
		voNeighbors.push_back(NeighborIndex);
	}
}

//*********************************************************************
//FUNCTION:
void Animation::CParticleSource::dumpOneRingNeighbors(int vParticleIndex, std::vector<int>& voNeighbors) const
{
	_ASSERTE(vParticleIndex >= 0 && vParticleIndex < m_ParticleNeighborList.size());
	const auto NeighborsSet = m_ParticleNeighborList[vParticleIndex].OneRingNeighbors;

	voNeighbors.reserve(NeighborsSet.size());
	for (auto NeighborIndex : NeighborsSet)
	{
		voNeighbors.push_back(NeighborIndex);
	}
}

//*********************************************************************
//FUNCTION:
void Animation::CParticleSource::__createParticlesFromVoxels()
{
	std::vector<int> Indices;
	std::vector<int> VoxelIndices;
	for (auto Itr = m_Voxels.begin(); Itr != m_Voxels.end(); Itr++)
	{
		Eigen::Vector3i Location = Itr->first;
		SVoxelInfo& VoxelInfo = Itr->second;

		VoxelInfo.RelatedParticles = std::vector<int>(VOXEL_RELATED_PARTICLES_COUNT);
		std::vector<int> ParticleIndices(VOXEL_RELATED_PARTICLES_COUNT);
		for (int i = 0; i < VOXEL_RELATED_PARTICLES_COUNT; i++)
		{
			Eigen::Vector3d ParticlePos = __calPariticleInitPos(VoxelInfo.CenterPos, m_VoxelInfo.Size, i);
			int CurParticleIndex = -1;
			if (!__isParticleCreated(Location, ParticlePos, CurParticleIndex))
			{
				SParticle Particle(ParticlePos);
				m_Particles.push_back(Particle);
				CurParticleIndex = m_Particles.size() - 1;
			}
			VoxelInfo.RelatedParticles[i] = CurParticleIndex;
			ParticleIndices[i] = CurParticleIndex;
			m_Particles[CurParticleIndex].RelatedVoxels.push_back(Location);
		}
		VoxelInfo.IsTracked = true;

		// Ensure that neighboring tetras are sharing faces
		if (VoxelInfo.SplitingCount % 2 == 1)
		{
			Indices.push_back(ParticleIndices[3]); Indices.push_back(ParticleIndices[2]); Indices.push_back(ParticleIndices[7]); Indices.push_back(ParticleIndices[1]);
			Indices.push_back(ParticleIndices[7]); Indices.push_back(ParticleIndices[1]); Indices.push_back(ParticleIndices[4]); Indices.push_back(ParticleIndices[5]);
			Indices.push_back(ParticleIndices[4]); Indices.push_back(ParticleIndices[2]); Indices.push_back(ParticleIndices[7]); Indices.push_back(ParticleIndices[6]);
			Indices.push_back(ParticleIndices[1]); Indices.push_back(ParticleIndices[2]); Indices.push_back(ParticleIndices[4]); Indices.push_back(ParticleIndices[0]);
			Indices.push_back(ParticleIndices[7]); Indices.push_back(ParticleIndices[2]); Indices.push_back(ParticleIndices[4]); Indices.push_back(ParticleIndices[1]);
		}
		else
		{
			Indices.push_back(ParticleIndices[0]); Indices.push_back(ParticleIndices[3]); Indices.push_back(ParticleIndices[6]); Indices.push_back(ParticleIndices[2]);
			Indices.push_back(ParticleIndices[5]); Indices.push_back(ParticleIndices[3]); Indices.push_back(ParticleIndices[0]); Indices.push_back(ParticleIndices[1]);
			Indices.push_back(ParticleIndices[6]); Indices.push_back(ParticleIndices[3]); Indices.push_back(ParticleIndices[5]); Indices.push_back(ParticleIndices[7]);
			Indices.push_back(ParticleIndices[5]); Indices.push_back(ParticleIndices[0]); Indices.push_back(ParticleIndices[6]); Indices.push_back(ParticleIndices[4]);
			Indices.push_back(ParticleIndices[0]); Indices.push_back(ParticleIndices[3]); Indices.push_back(ParticleIndices[5]); Indices.push_back(ParticleIndices[6]);
		}
	}

	m_TetModel = boost::shared_ptr<CTetModel>(new CTetModel());
	m_TetModel->initializeMesh(Indices);
}

//*********************************************************************
//FUNCTION:
Eigen::Vector3d Animation::CParticleSource::__calPariticleInitPos(const Eigen::Vector3d& vVoxelCenterPos, double vVoxelSize, int vLocalParticleIndex) const
{
	_ASSERTE(vLocalParticleIndex >= 0 && vLocalParticleIndex <= VOXEL_RELATED_PARTICLES_COUNT);

	Eigen::Vector3d Offset;
	switch (vLocalParticleIndex)
	{
	case 0:
		Offset[0] = -vVoxelSize / 2.0f;
		Offset[1] = -vVoxelSize / 2.0f;
		Offset[2] = -vVoxelSize / 2.0f;
		break;
	case 1:
		Offset[0] = vVoxelSize / 2.0f;
		Offset[1] = -vVoxelSize / 2.0f;
		Offset[2] = -vVoxelSize / 2.0f;
		break;
	case 2:
		Offset[0] = -vVoxelSize / 2.0f;
		Offset[1] = -vVoxelSize / 2.0f;
		Offset[2] = vVoxelSize / 2.0f;
		break;
	case 3:
		Offset[0] = vVoxelSize / 2.0f;
		Offset[1] = -vVoxelSize / 2.0f;
		Offset[2] = vVoxelSize / 2.0f;
		break;
	case 4:
		Offset[0] = -vVoxelSize / 2.0f;
		Offset[1] = vVoxelSize / 2.0f;
		Offset[2] = -vVoxelSize / 2.0f;
		break;
	case 5:
		Offset[0] = vVoxelSize / 2.0f;
		Offset[1] = vVoxelSize / 2.0f;
		Offset[2] = -vVoxelSize / 2.0f;
		break;
	case 6:
		Offset[0] = -vVoxelSize / 2.0f;
		Offset[1] = vVoxelSize / 2.0f;
		Offset[2] = vVoxelSize / 2.0f;
		break;
	case 7:
		Offset[0] = vVoxelSize / 2.0f;
		Offset[1] = vVoxelSize / 2.0f;
		Offset[2] = vVoxelSize / 2.0f;
		break;
	default:
		throw std::runtime_error("Invalid paricle's number.");
		break;
	}

	return vVoxelCenterPos + Offset;
}

//*********************************************************************
//FUNCTION:
bool Animation::CParticleSource::__isParticleCreated(const Eigen::Vector3i& vLocation, const Eigen::Vector3d& vParticlePos, int& voParticleIndex)
{
	Eigen::Vector3i NeighborLocation;

	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			for (int z = -1; z <= 1; z++)
			{
				int Count = z + y;
				if (x == 0 && y == 0 && z == 0)
					continue;

				NeighborLocation = { vLocation[0] + x, vLocation[1] + y, vLocation[2] + z };
				auto Itr = m_Voxels.find(NeighborLocation);
				if (Itr != m_Voxels.end())
				{
					SVoxelInfo& VoxelInfo = Itr->second;
					if (VoxelInfo.IsTracked)
					{
						std::vector<int>& RelatedParticles = VoxelInfo.RelatedParticles;
						for (int i = 0; i < MAX_RELATED_VOXEL; i++)
						{
							Eigen::Vector3d Offset = m_Particles[RelatedParticles[i]].CenterPos - vParticlePos;
							if (fabs(Offset[0]) < DBL_EPSILON && fabs(Offset[1]) < DBL_EPSILON && fabs(Offset[2]) < DBL_EPSILON)
							{
								voParticleIndex = RelatedParticles[i];
								return true;
							}
						}
					}
				}
			}
		}
	}
	return false;
}

//*********************************************************************
//FUNCTION:
void Animation::CParticleSource::__generateOneRingNeighbors()
{
	_ASSERTE(m_Particles.size() > 0);
	m_ParticleNeighborList.resize(m_Particles.size());

	const int ParticleCount = m_Particles.size();
	for (int i = 0; i < ParticleCount; i++)
	{
		auto RelatedVoxels = m_Particles[i].RelatedVoxels;
		const int RelatedVoxelCount = RelatedVoxels.size();
		if (RelatedVoxelCount < MAX_RELATED_VOXEL)
		{
			m_Particles[i].IsSkin = true;
			m_SkinParticleIndices.push_back(i);
		}

		for (int k = 0; k < RelatedVoxelCount; k++)
		{
			auto VoxelInfo = m_Voxels.at(RelatedVoxels[k]);

			_ASSERTE(!VoxelInfo.RelatedParticles.empty());
			for (int m = 0; m < VOXEL_RELATED_PARTICLES_COUNT; m++)
			{
				const int NeighborIndex = VoxelInfo.RelatedParticles[m];

				if (NeighborIndex != i)
				{
					m_ParticleNeighborList[i].OneRingNeighbors.insert(NeighborIndex);
					m_ParticleNeighborList[NeighborIndex].OneRingNeighbors.insert(i);
				}
			}
		}
	}
}

//*********************************************************************
//FUNCTION:
void Animation::CParticleSource::__generateCommonNeighbors()
{
	const int ParticleCount = m_Particles.size();
	const double VoxelSize = m_VoxelInfo.Size;
#pragma omp parallel for
	for (int i = 0; i < ParticleCount; i++)
	{
		std::set<int> OneRingNeighbor = m_ParticleNeighborList[i].OneRingNeighbors;
		for (auto Itr = OneRingNeighbor.begin(); Itr != OneRingNeighbor.end(); Itr++)
		{
			const int Index = *Itr;
			Eigen::Vector3d Offset = m_Particles[i].CenterPos - m_Particles[Index].CenterPos;
			if (fabs(Offset.norm() - VoxelSize) < DBL_EPSILON)
				m_ParticleNeighborList[i].CommonNeighbors.insert(Index);
		}
	}
}

//*********************************************************************
//FUNCTION:
void Animation::CParticleSource::__generateRegions()
{
	const int ParticleCount = m_Particles.size();
	std::vector<int> NumOfRelatedRegions(ParticleCount, 0);
	for (int i = 0; i < ParticleCount; i++)
	{
		__createRegion(i, NumOfRelatedRegions);
	}

	const int RegionCount = m_Regions.size();
#pragma omp parallel for
	for (int i = 0; i < RegionCount; i++)
	{
		const int CurRegionSize = m_Regions[i].RegionParticles.size();
		std::vector<int> NumOfRelatedRegionsInParticle(CurRegionSize);
		for (int k = 0; k < CurRegionSize; k++)
		{
			const int ParticleIndex = m_Regions[i].RegionParticles[k];
			NumOfRelatedRegionsInParticle[k] = NumOfRelatedRegions[ParticleIndex];
		}

		m_Regions[i].NumOfRelatedRegionsInParticle = NumOfRelatedRegionsInParticle;
	}
}

//*********************************************************************
//FUNCTION:
void Animation::CParticleSource::__createRegion(int vIndex, std::vector<int>& vioNumOfRelatedRegions)
{
	std::vector<int> RegionParticles;
	RegionParticles.push_back(vIndex);
	for (auto Itr = m_ParticleNeighborList[vIndex].OneRingNeighbors.begin(); Itr != m_ParticleNeighborList[vIndex].OneRingNeighbors.end(); Itr++)
	{
		RegionParticles.push_back(*Itr);
	}

	bool IsAddNewRegion = true;
	const int RegionCount = m_Regions.size();
	for (int k = 0; k < RegionCount; k++)
	{
		if (__isRegionCreated(RegionParticles, m_Regions[k].RegionParticles))
		{
			IsAddNewRegion = false;
			break;
		}
	}

	if (IsAddNewRegion)
	{
		SRegion Region;
		Region.RegionParticles = RegionParticles;
		m_Regions.push_back(Region);
		const int RegionSize = RegionParticles.size();
		for (int k = 0; k < RegionSize; k++)
		{
			const int Index = RegionParticles[k];
			vioNumOfRelatedRegions[Index] += 1;
		}
	}
}

//*********************************************************************
//FUNCTION:
bool Animation::CParticleSource::__isRegionCreated(const std::vector<int>& vLeft, std::vector<int>& vRight)
{
	/*if (vLeft.size() != vRight.size())
		return false;

	for (int i = 0; i < vLeft.size(); i++)
	{
		auto Itr = find(vRight.begin(), vRight.end(), vLeft[i]);
		if (Itr == vRight.end())
		{
			return false;
		}
	}

	return true;*/

	std::vector<int> temp;
	int count = 0;
	for (int i = 0; i < vLeft.size(); i++)
	{
		auto Itr = find(vRight.begin(), vRight.end(), vLeft[i]);
		if (Itr != vRight.end())
			count++;
		else
			temp.push_back(vLeft[i]);
	}

	if (vLeft.size() > vRight.size() && count == vRight.size())
	{
		vRight.insert(vRight.end(), temp.begin(), temp.end());
		return true;
	}

	if (vLeft.size() <= vRight.size() && count == vLeft.size())
	{
		return true;
	}

	return false;
}