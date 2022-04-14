#pragma once
#include "Common.h"
#include <unordered_map>
#include <boost/shared_ptr.hpp>
#include "TetModel.h"

namespace Animation
{
	class CParticleSource
	{
	public:
		CParticleSource() = default;
		virtual ~CParticleSource() = default;

		void initialize();
		void setVoxelizationInfo(const SVoxelizationInfo& vVoxelizationInfo);
		void addVoxel(int vSplitingCount, Eigen::Vector3i vLocation, Eigen::Vector3d vVoxelPos);
		std::vector<int> findNearestVertex(const Eigen::Vector3d& vPosition);

		Eigen::Vector3i getBoxExtendSize() const { return m_VoxelInfo.FilledVoxelSize; }
		double getParticleRadius() const { return m_VoxelInfo.Size / 2; }
		int getParticleCount() const { return m_Particles.size(); }
		Eigen::Vector3d getParticlePos(int vIndex) const { _ASSERTE(vIndex >= 0 && vIndex < m_Particles.size()); return m_Particles[vIndex].CenterPos; }
		int getRegionCount() const { return m_Regions.size(); }
		SRegion getRegion(int vIndex) const { _ASSERTE(vIndex >= 0 && vIndex < m_Regions.size()); return m_Regions[vIndex]; }
		int getSkinParticleCount() const { return m_SkinParticleIndices.size(); }
		bool isSkinParticle(int vIndex) const { _ASSERTE(vIndex >= 0 && vIndex < m_Particles.size()); m_Particles[vIndex].IsSkin; }
		const CTetModel& getTetModel() const { return *m_TetModel; }
		void dumpSkinParticleIndices(std::vector<int>& voSkinList) const { voSkinList = m_SkinParticleIndices; }
		void dumpCommonNeighbors(int vParticleIndex, std::vector<int>& voNeighbors) const;
		void dumpOneRingNeighbors(int vParticleIndex, std::vector<int>& voNeighbors) const;

#ifdef _UNIT_TEST
		int getVoxelCount() const { return m_Voxels.size(); }
		SVoxelInfo getVoxelInfo(const Eigen::Vector3i& vLocation) const { return m_Voxels.at(vLocation); }
		Eigen::Vector3d getParticleInitPos(const Eigen::Vector3d& vCenterPos, double vSize, int vParticleIndex) const { return __calPariticleInitPos(vCenterPos, vSize, vParticleIndex); }
		void createParticlesFromVoxels() { __createParticlesFromVoxels(); }
		Eigen::Vector3i getVoxelLocation(int vIndex) const { _ASSERTE(vIndex >= 0 && vIndex < m_Voxels.size()); auto Itr = m_Voxels.begin(); std::advance(Itr, vIndex); return Itr->first; }
		SParticle getParticle(int vIndex) const { _ASSERTE(vIndex >= 0 && vIndex < m_Particles.size()); return m_Particles[vIndex]; }
		void generateOneRingNeighbors() { __generateOneRingNeighbors(); }
		void generateCommonNeighbors() { __generateCommonNeighbors(); }
		int getTetSize() const { return m_TetModel->getTetSize(); }
		bool isSameParticles(const std::vector<int>& vLeft, const std::vector<int>& vRight) {  return __isRegionCreated(vLeft, vRight); }
#endif // _UNIT_TEST

	private:
		void __createParticlesFromVoxels();
		Eigen::Vector3d __calPariticleInitPos(const Eigen::Vector3d& vVoxelCenterPos, double vVoxelSize, int vLocalParticleIndex) const;
		bool __isParticleCreated(const Eigen::Vector3i& vLocation, const Eigen::Vector3d& vParticlePos, int& voParticleIndex);
		void __generateOneRingNeighbors();
		void __generateCommonNeighbors();
		void __generateRegions();
		void __createRegion(int vIndex, std::vector<int>& vioNumOfRelatedRegions);
		bool __isRegionCreated(const std::vector<int>& vLeft, std::vector<int>& vRight);

		std::unordered_map<Eigen::Vector3i, SVoxelInfo, HashFuncVector3i, EqualKeyVector3i> m_Voxels; //key: Index in voxel's grid
		std::vector<SParticle> m_Particles;
		SVoxelizationInfo m_VoxelInfo;
		boost::shared_ptr<CTetModel> m_TetModel;
		std::vector<SParticleNeighbor> m_ParticleNeighborList;
		std::vector<SRegion> m_Regions;
		std::vector<int> m_SkinParticleIndices;
	};
}