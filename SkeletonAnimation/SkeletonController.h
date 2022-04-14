#pragma once
#include "Common.h"

namespace Animation
{
	class CSkeletonController
	{
	public:
		CSkeletonController() = default;
		explicit CSkeletonController(const std::string& vFilePath);
		virtual ~CSkeletonController() = default;

		bool initSkeletonByFile(const std::string& vFilePath);
		bool dumpNextPose(std::vector<Eigen::Matrix4d>& voSkeletonTransfer);
		void resetPose();

		int getBoneCount() const { return m_Skeleton.size(); }
		int getParentIndexAt(int vIndex) const;
		const Eigen::Vector3d getInitialBoneWorldPos(int vIndex) const;
		const Eigen::Matrix4d getWorldToLocalTransfer(int vIndex) const;

#ifdef _UNIT_TEST
		std::vector<SSkeletonDefinition> getSkeleton() const { return m_Skeleton; }
		std::vector<std::vector<Eigen::Matrix4d>> getAllPose() const { return m_AllPose; }
		std::vector<Eigen::Vector3d> getInitialWorldPosition() const { return m_InitialWorldPos; }
		std::vector<Eigen::Matrix4d> getWorldToLocalTransfer() const { return m_WorldToLocalTransfer; }
#endif // _UNIT_TEST

	private:
		void __generateSkeleton(const std::vector<std::string>& vAllFileStr);
		void __generateAllPose(const std::vector<std::string>& vAllFileStr);
		void __generateInitialData();

		std::vector<SSkeletonDefinition> m_Skeleton;
		std::vector<std::vector<Eigen::Matrix4d>> m_AllPose;
		std::vector<Eigen::Vector3d> m_InitialWorldPos;
		std::vector<Eigen::Matrix4d> m_WorldToLocalTransfer;
		int m_CurrentPoseIndex = -1;
	};
}
