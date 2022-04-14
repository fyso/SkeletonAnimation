#pragma once
#include "ParticleSource.h"
#include "SkeletonController.h"
#include "UndirectedGraph.h"

namespace Animation
{
	class CParticleLayer
	{
	public:
		CParticleLayer() = default;
		virtual ~CParticleLayer();

		void initialize(CParticleSource* vParticleSource, CSkeletonController* vSkeletonController);
		void dumpBoneParticles(std::vector<SBoneParticle>& voBoneLayer) const 
		{ 
			voBoneLayer = m_BoneLayer;
		}
		void dumpSkinParticles(std::vector<SSkinParticle>& voSkinLayer) const { voSkinLayer = m_SkinLayer; }
		double getParticleStiffness(int vIndex) const;
		Eigen::Vector3f getParticleColor(int vIndex) const;
		bool isBoneLayer(int vIndex) const;

#ifdef _UNIT_TEST
		bool isIntersect(Eigen::Vector3d vBoneStartPos, Eigen::Vector3d vBoneEndPos, Eigen::Vector3d vParticlePos, double vParticleRadius)
		{
			return __isIntersect(vBoneStartPos, vBoneEndPos, vParticlePos, vParticleRadius);
		}
		void sortDistance(std::vector<int> vSource, std::vector<int>& voResult)
		{
			__sortDistance(vSource, voResult);
		}
#endif // _UNIT_TEST

	private:
		void __clear();
		void __initGraph();
		void __initBoneLayer();
		void __generateBindInfo();
		void __generateSkinLayer();
		void __generateMuscleAndFatLayer();
		void __sortDistance(std::vector<int> vSource, std::vector<int>& voResult);
		float __calculateBindWeight(float vBoxExtend, float vDistance);
		bool __isIntersect(Eigen::Vector3d vBoneStartPos, Eigen::Vector3d vBoneEndPos, Eigen::Vector3d vParticlePos, double vParticleRadius);
		bool __isInVoxel(const Eigen::Vector3d& vPos, const Eigen::Vector3d& vMaxCorner, const Eigen::Vector3d& vMinCorner) const;

		double m_SmoothParameter = 0.0;
		CParticleSource* m_pParticleSource = nullptr;
		CSkeletonController* m_pSkeletonController = nullptr;
		CUndirectedGraph* m_pGraph = nullptr;
		std::vector<SBoneSegment> m_BoneSegments;
		std::vector<SBoneParticle> m_BoneLayer;
		std::vector<SSkinParticle> m_SkinLayer;
		std::vector<double> m_StiffnessValue;
		std::vector<double> m_ParticleStiffness;
		std::vector<Eigen::Vector3f> m_ParticleColor;
	};
}
