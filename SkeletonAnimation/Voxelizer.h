#pragma once
#include <queue>
#include "Model.h"
#include "Shader.h"
#include <boost/dynamic_bitset.hpp>
#include <boost/shared_ptr.hpp>
#include "Common.h"
#include "ParticleSource.h"

namespace Animation
{
	class CVoxelizer
	{
	public:
		CVoxelizer() = default;
		virtual ~CVoxelizer() = default;

		void voxelize(const std::string& vModelFilePath);
		void sendVoxels(CParticleSource& vParticleSource);
		bool loadVoxelData(const std::string& vVoxelDataFilePath);
		void saveVoxelData(const std::string& vVoxelDataFilePath);

#ifdef _UNIT_TEST
		const boost::dynamic_bitset<>& getVoxelStatusSet() const { return m_VoxelStatusSet; }
		glm::ivec3 getVoxelResolution() const { return m_VoxelResolution; }
		Animation::SAABB getModelAABB() const { return m_ModelAABB; }
#endif

	private:
		void __init(const std::string& vModelFilePath);
		void __calculateModelAABB();
		void __voxelizeModelSurface();
		void __voxelizeModelInterior();
		void __floodVoxelStatusSet(std::queue<glm::ivec3>& vioVoxelQueue);
		void __setupCountBuffer(GLuint& voCountBuffer, int vVoxelSetLength);
		void __setupShader(graphics::CShader& vioShader, const glm::ivec3& vResolution);
		void __tagSurfaceVoxels(const graphics::CShader& vShader, GLuint vCountBuffer, int vVoxelSetLength);
		std::vector<SCameraInfo> __generateCamerasPlacedAlongAxes() const;
		inline bool __isValidVoxel(glm::ivec3 vPoint) const;
		inline int __transformIndex2Int(const glm::ivec3& vIndex) const;

		void __generateVoxelsInfo(const std::vector<std::string>& vAllStrFromFile);

		float m_VoxelSize = 0.0f;
		float m_AABBPadding = 0.0f;
		glm::ivec3 m_VoxelResolution;
		boost::shared_ptr<graphics::CModel> m_pModel;
		SAABB m_ModelAABB;
		boost::dynamic_bitset<> m_VoxelStatusSet;
	};
}