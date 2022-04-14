#pragma once
#include "DistanceConstraint.h"
#include "VolumeConstraint.h"
#include "ShapeMatchingConstraint.h"
#include "ProjectionConstraint.h"
#include "UndirectedGraph.h"
#include "ParticleSource.h"
#include "ParticleLayer.h"
#include "Common.h"
#include "ColorableGraph.h"
#include <boost/shared_ptr.hpp>
#include <map>
#include <algorithm>

namespace Animation
{
	class CSimulationModel
	{
		friend class boost::serialization::access;
	public:
		CSimulationModel() = default;
		virtual ~CSimulationModel() = default;
		void generateConstraint(
			boost::shared_ptr<CParticleSource> vParticleSource,
			const boost::shared_ptr<CParticleLayer> vParticleLayer,
			const EConstraintsType vConstraintsType,
			int vParallelNumber = 10
		);


		void addSimulationParticle(const Eigen::Vector3d& vPos, const Eigen::Vector3d& vGravity);
		void resetSimulationState();

		void dumpSimulationParticle(std::vector<CParticleData>& voConstraints) const;
		void dumpDistanceConstraints(std::vector<CDistanceConstraint>& voConstraints) const;
		void dumpDistanceProjectionConstraints(std::vector<CProjectionConstraint>& voConstraints) const;
		void dumpVolumeConstraints(std::vector<CVolumeConstraint>& voConstraints) const;
		void dumpVolumeProjectionConstraints(std::vector<CProjectionConstraint>& voConstraints) const;
		void dumpShapeMatchingConstraints(std::vector<CShapeMatchingConstraint>& voConstraints) const;
		void dumpShapeMatchingProjectionConstraints(std::vector<CProjectionConstraint>& voConstraints) const;

		void setParticleMass(int vIndex, double vMass);
		void setDecoplingRate(int vInput) { if (vInput > 100) vInput = 100; if (vInput < 0)vInput = 0; m_DecoplingRate = vInput; }
		int getVolumeDuplicateparticleNumber() const;
		int getShapeMatchingDuplicateparticleNumber() const;
		int getDistanceDuplicateparticleNumber() const;
		int getVolumeProjectionConstraintsNumber() const { return m_VolumeProjectionConstraints.size(); };
		int getDistanceProjectionConstraintsNumber() const { return m_DistanceProjectionConstraints.size(); };
		int getShapeMatchingProjectionConstraintsNumber() const { return m_ShapeMatchingProjectionConstraints.size(); };
		int getDistanceConstraintsSize() const { return m_DistanceConstraints.size(); }
		int getVolumeConstraintsSize() const { return m_VolumeConstraints.size(); }
		int getShapeMatchingConstraintsSize() const { return m_ShapeMatchingConstraints.size(); }
		const std::vector<int> getDistanceConstraintsGroupOffset() const { return m_DistanceConstraintsGroupOffset; }
		const std::vector<int> getShapeMatchingConstraintsGroupOffset() const { return m_ShapeMatchingConstraintsGroupOffset; }
		const std::vector<int> getVolumeConstraintsGroupOffset() const { return m_VolumeConstraintsGroupOffset; }
		const std::vector<int> getParticleRelatedClusterCount() const { return m_ParticleRelatedClusterCount; }

	private:
		std::vector<CParticleData> m_ParticleData;
		int m_DecoplingRate = 10;
		int m_CountOfRealParticle = 0;

		std::vector<CDistanceConstraint> m_DistanceConstraints;
		std::vector<CVolumeConstraint> m_VolumeConstraints;
		std::vector<CShapeMatchingConstraint> m_ShapeMatchingConstraints;

		std::map<int, std::vector<int>> m_DistanceDuplicateParticles;
		std::map<int, std::vector<int>> m_VolumeDuplicateParticles;
		std::map<int, std::vector<int>> m_ShapeMatchingDuplicateParticles;

		std::vector<CProjectionConstraint> m_VolumeProjectionConstraints;
		std::vector<CProjectionConstraint> m_ShapeMatchingProjectionConstraints;
		std::vector<CProjectionConstraint> m_DistanceProjectionConstraints;

		std::vector<int> m_DistanceConstraintsGroupOffset;
		std::vector<int> m_VolumeConstraintsGroupOffset;
		std::vector<int> m_ShapeMatchingConstraintsGroupOffset;

		std::vector<int> m_ParticleRelatedClusterCount;
		std::vector<int> m_DistanceMapCount;
		std::vector<int> m_VolumeMapCount;

		std::vector<std::vector<int>> __splitConstraintsWithLDF(
			const std::vector<std::vector<int>>& vSourceConstraints,
			int vMaxParallelNumber,
			const EConstraintsType vConstraintsType
		);
		std::vector<std::vector<int>> __splitConstraintsWithLEF(
			const std::vector<std::vector<int>>& vSourceConstraints,
			int vMaxParallelNumber,
			const EConstraintsType vConstraintsType
		);
		std::vector<std::vector<int>> __splitConstraintsWithLCF(
			const std::vector<std::vector<int>>& vSourceConstraints,
			int vMaxParallelNumber,
			const EConstraintsType vConstraintsType
		);
		void __generateShapeMatchingConstraint(
			boost::shared_ptr<CParticleSource> vParticleSource,
			const boost::shared_ptr<CParticleLayer> vParticleLayer,
			int vParallelNumber
		);
		void __generateVolumeConstraint(
			const boost::shared_ptr<CParticleSource> vParticleSource,
			const boost::shared_ptr<CParticleLayer> vParticleLayer,
			int vParallelNumber
		);
		void __generateDistanceConstraint(
			const boost::shared_ptr<CParticleSource> vParticleSource,
			const boost::shared_ptr<CParticleLayer> vParticleLayer,
			int vParallelNumber
		);

		void __generateVolumeProjectionConstraints();
		void __generateDistanceProjectionConstraints();
		void __generateShapeMatchingProjectionConstraints();

		void __deleteEdgesInClique(const std::vector<int>& vInputClique, int vColorNumberQ, EConstraintsType vConstraintType, CColorableGraphPtr voColorGraph);
		void __generateColorableGraph(CColorableGraphPtr voColorGraph, int vParticlesCount, const std::vector<std::vector<int>>& vTargetConstraints);
		bool __ifTwoConstraintConnected(const std::map<int, std::vector<int>>& vParticlesConstraionMap, int vConstraintIdx0, int vConstraintIdx1);

		void __generateDistanceDuplicateParticles(const std::vector<int>& vInputClique, int vTargetParticleIndex);
		void __generateVolumeDuplicateParticles(const std::vector<int>& vInputClique, int vTargetParticleIndex);
		void __generateShapeMatchingDuplicateParticles(const std::vector<int>& vInputClique, int vTargetParticleIndex);
		void __writeGraphToFile(const std::vector<std::vector<int>>& vGraph, const std::string& vCacheFileName);
		template<typename Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			ar & m_ParticleData;
			ar & m_DistanceConstraints;
			ar & m_ShapeMatchingConstraints;
			ar & m_VolumeConstraints;
			ar & m_DistanceProjectionConstraints;
			ar & m_ShapeMatchingProjectionConstraints;
			ar & m_VolumeProjectionConstraints;
			ar & m_DistanceConstraintsGroupOffset;
			ar & m_ShapeMatchingConstraintsGroupOffset;
			ar & m_VolumeConstraintsGroupOffset;
			ar & m_ParticleRelatedClusterCount;
		}
	};
}