#pragma once
#include <glm/glm.hpp>
#include <algorithm>
#include <Eigen/Dense>
#include "Mesh.h"
#include <map>
#include <set>
#include <boost/archive/text_oarchive.hpp> 
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>

namespace Animation
{
	enum EConstraintsType
	{
		DISTANCE = 0,
		VOLUME,
		SHAPEMATCHING
	};

	constexpr int MAX_RELATED_VOXEL = 8;
	constexpr int MAX_DISTANCE = 100000;
	constexpr int INFLUENCE_NUM = 4;

	struct SVoxelizationInfo
	{
		Eigen::Vector3i TotalVoxelSize;
		Eigen::Vector3i FilledVoxelSize;
		Eigen::Vector3d StartPos;
		double Size;
	};

	constexpr int VOXEL_RELATED_PARTICLES_COUNT = 8;
	struct SVoxelInfo
	{
		Eigen::Vector3d CenterPos;
		int SplitingCount = 0;
		std::vector<int> RelatedParticles; //一个Voxel对应的8个Particle的索引
		bool IsTracked = false; //用来判断是否已经生成过粒子

		SVoxelInfo(Eigen::Vector3d vPos) : CenterPos(vPos) {};
	};

	struct SParticle
	{
		Eigen::Vector3d CenterPos;
		std::vector<Eigen::Vector3i> RelatedVoxels; // 如果是内部粒子，包含这个粒子的Voxel为8个。边界处小于8个。
		bool IsSkin = false;

		SParticle() = default;
		explicit SParticle(const Eigen::Vector3d& vPos) : CenterPos(vPos) {}
	};

	struct SParticleNeighbor
	{
		std::set<int> CommonNeighbors; //x+-,y+-,z+-方向的邻居，最多为6个
		std::set<int> OneRingNeighbors; //共享一个Voxel的所有邻居，最多为26个
	};

	struct SRegion
	{
		std::vector<int> RegionParticles;
		std::vector<int> NumOfRelatedRegionsInParticle;
	};

	struct SCameraInfo
	{
		glm::vec3 Eye;
		glm::vec3 LookAt;
		glm::vec3 Up;
	};

	struct SAABB
	{
		glm::vec3 MaxCoordinate = glm::vec3(-FLT_MAX);
		glm::vec3 MinCoordinate = glm::vec3(FLT_MAX);

		void updateAABB(const graphics::SVertex& vVertex)
		{
			MinCoordinate.x = std::min(vVertex.Position.x, MinCoordinate.x);
			MinCoordinate.y = std::min(vVertex.Position.y, MinCoordinate.y);
			MinCoordinate.z = std::min(vVertex.Position.z, MinCoordinate.z);
			MaxCoordinate.x = std::max(vVertex.Position.x, MaxCoordinate.x);
			MaxCoordinate.y = std::max(vVertex.Position.y, MaxCoordinate.y);
			MaxCoordinate.z = std::max(vVertex.Position.z, MaxCoordinate.z);
		}
	};

	struct SSkeletonDefinition
	{
		int ParentIndex = -1;
		int BoneID = -1;
		Eigen::Vector3d Offset;

		bool operator == (const SSkeletonDefinition& rhs) const
		{
			return ParentIndex == rhs.ParentIndex && BoneID == rhs.BoneID
				&& (fabs(Offset.x() - rhs.Offset.x()) < FLT_EPSILON)
				&& (fabs(Offset.y() - rhs.Offset.y()) < FLT_EPSILON)
				&& (fabs(Offset.z() - rhs.Offset.z()) < FLT_EPSILON);
		}

		bool operator != (const SSkeletonDefinition& rhs) const
		{
			return !(*this == rhs);
		}
	};

	enum ParticleType : int
	{
		BONE = 0, MUSCLE = 1, FAT = 2, SKIN = 3
	};

	struct SBoneSegment
	{
		int StartBoneIndex = -1; //parent bone
		int EndBoneIndex = -1;
		std::vector<int> IntersetParticles;

		SBoneSegment() {}

		SBoneSegment(int vStartBoneIndex, int vEndBoneIndex) : StartBoneIndex(vStartBoneIndex), EndBoneIndex(vEndBoneIndex) {}
	};


	struct SBindInfo
	{
		int BoneIndex = -1; //绑定的关节索引
		double Weight = 0.0;
		Eigen::Vector3d LocalPosition;
	};

	struct SBoneParticle
	{
		int ParticleIndex = -1;
		SBindInfo BindInfoList[INFLUENCE_NUM];

		SBoneParticle() = default;
		SBoneParticle(int vParticleIndex) : ParticleIndex(vParticleIndex) {}

		bool operator == (const SBoneParticle& rhs) const
		{
			return ParticleIndex == rhs.ParticleIndex;
		}

		bool operator != (const SBoneParticle& rhs) const
		{
			return ParticleIndex != rhs.ParticleIndex;
		}
	};

	struct SSkinParticle
	{
		int ParticleIndex = -1;
		int MaxDistanceToSkin = MAX_DISTANCE; //所有粒子到当前皮肤粒子的最远距离

		SSkinParticle() = default;
		SSkinParticle(int vParticleIndex) : ParticleIndex(vParticleIndex) {}
	};

	struct SMeshVertex
	{
		Eigen::Vector3d OriginPosition;
		Eigen::Vector3d Position;
		std::vector<int> RelatedParticleIndices;
	};

	struct HashFuncVector3i
	{
		std::size_t operator()(const Eigen::Vector3i& vKey) const
		{
			return ((std::hash<int>()(vKey[0])
				^ (std::hash<int>()(vKey[1]) << 1)) >> 1)
				^ (std::hash<int>()(vKey[2]) << 1);
		}
	};

	struct EqualKeyVector3i
	{
		bool operator()(const Eigen::Vector3i& vLeft, const Eigen::Vector3i& vRight) const
		{
			return vLeft[0] == vRight[0] && vLeft[1] == vRight[1] && vLeft[2] == vRight[2];
		}
	};

	inline Eigen::Vector3d translatePostion(Eigen::Vector3d vParticlePos, Eigen::Matrix4d vTransferMatrix)
	{
		Eigen::Vector4d Vector(vParticlePos[0], vParticlePos[1], vParticlePos[2], 1.0f);
		Eigen::Vector4d Result = vTransferMatrix * Vector;
		Eigen::Vector3d NewPosition(Result[0], Result[1], Result[2]);
		return NewPosition;
	}


	static std::vector<int> computeIntersectionOfTwoVector(const std::vector<int> vioLeft, const std::vector<int>& vRight)
	{
		std::vector<int> Temp;
		for (int i = 0; i < vioLeft.size(); i++)
		{
			if (find(vRight.begin(), vRight.end(), vioLeft[i]) != vRight.end())
			{
				Temp.push_back(vioLeft[i]);
			}
		}
		return Temp;
	}

	static std::vector<int> computeUnionOfVectorWithInt(const std::vector<int> vioLeft, int vRight)
	{
		std::vector<int> Temp = vioLeft;
		if (find(vioLeft.begin(), vioLeft.end(), vRight) == vioLeft.end())
		{
			Temp.push_back(vRight);
		}
		return Temp;
	}

}