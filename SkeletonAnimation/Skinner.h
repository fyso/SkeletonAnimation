#pragma once
#include "Model.h"
#include "Shader.h"
#include "ParticleSource.h"

namespace Animation
{
	class CSkinner
	{
	public:
		CSkinner(graphics::CModel *vModel, CParticleSource *vParticleSource);
		~CSkinner();

		void draw(graphics::CShader vShader);
		void updateAndDraw(graphics::CShader vShader, const std::vector<Eigen::Vector3d>& vPositions);

	private:
		void __initMeshVertices();
		void __updateVertices(const std::vector<Eigen::Vector3d>& vPositions);

		double m_AngleThreshold = -1.0;
		boost::shared_ptr<graphics::CModel> m_pModel = nullptr;
		CParticleSource *m_pParticleSource = nullptr;
		std::vector<std::vector<SMeshVertex>> m_MeshVertices;
	};
}