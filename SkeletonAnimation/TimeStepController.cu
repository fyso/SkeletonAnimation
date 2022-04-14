#pragma once
#include "TimeStepController.h"
#include "PositionBasedDynamics.cu"
#include <iostream>

int *dBoneIndices = nullptr;
Animation::SVector3d *dBonePos = nullptr;
Animation::SVector3d *dParticlePos = nullptr;

Animation::CTimeStepController::CTimeStepController(int vIterationCount, double vTimeStep, double vDampFactor)
	: m_IterationCount(vIterationCount), m_TimeStep(vTimeStep), m_DampFactor(vDampFactor)
{
}

Animation::CTimeStepController::~CTimeStepController()
{
	__freeCudaMemory();
}

//*********************************************************************
//FUNCTION:
void Animation::CTimeStepController::init(CSimulationModel& vSimulationModel, int vOriginParticleCount)
{
	m_OriginParticleCount = vOriginParticleCount;

	cudaDeviceProp DeviceProp;
	HANDLE_ERROR(cudaGetDeviceProperties(&DeviceProp, 0));
	m_MaxThreadsPerBlock = DeviceProp.maxThreadsPerBlock / 4;
	HANDLE_ERROR(cudaSetDevice(0));

	std::vector<CParticleData> ParticleData;
	vSimulationModel.dumpSimulationParticle(ParticleData);
	m_ParticleCount = ParticleData.size();

	int Size = m_ParticleCount * sizeof(CParticleData);
	HANDLE_ERROR(cudaMalloc((void**)&m_dParticleData, Size));
	HANDLE_ERROR(cudaMemcpy(m_dParticleData, ParticleData.data(), Size, cudaMemcpyHostToDevice));

	Size = m_OriginParticleCount * sizeof(SVector3d);
	HANDLE_ERROR(cudaMalloc((void**)&dParticlePos, Size));

	std::vector<CDistanceConstraint> DistanceConstraints;
	vSimulationModel.dumpDistanceConstraints(DistanceConstraints);
	m_DistanceConstraintCount = DistanceConstraints.size();
	if (m_DistanceConstraintCount > 0)
	{
		m_DistanceConstraintOffset.push_back(0);
		std::vector<int> DistanceConstraintsGroupTemp = vSimulationModel.getDistanceConstraintsGroupOffset();
		m_DistanceConstraintOffset.insert(m_DistanceConstraintOffset.end(), DistanceConstraintsGroupTemp.begin(), DistanceConstraintsGroupTemp.end());
		Size = m_DistanceConstraintCount * sizeof(CDistanceConstraint);
		HANDLE_ERROR(cudaMalloc((void**)&m_dDistanceConstraints, Size));
		HANDLE_ERROR(cudaMemcpy(m_dDistanceConstraints, DistanceConstraints.data(), Size, cudaMemcpyHostToDevice));
	}
	
	std::vector<CProjectionConstraint> DistanceProjectionConstraints;
	vSimulationModel.dumpDistanceProjectionConstraints(DistanceProjectionConstraints);
	m_DistanceProjectionConstraintCount = DistanceProjectionConstraints.size();
	if (m_DistanceProjectionConstraintCount > 0)
	{
		Size = m_DistanceProjectionConstraintCount * sizeof(CProjectionConstraint);
		HANDLE_ERROR(cudaMalloc((void**)&m_dDistanceProjectionConstraints, Size));
		HANDLE_ERROR(cudaMemcpy(m_dDistanceProjectionConstraints, DistanceProjectionConstraints.data(), Size, cudaMemcpyHostToDevice));
	}	

	std::vector<CVolumeConstraint> VolumeConstraints;
	vSimulationModel.dumpVolumeConstraints(VolumeConstraints);
	m_VolumeConstraintCount = VolumeConstraints.size();
	if (m_VolumeConstraintCount > 0)
	{
		m_VolumeConstraintOffset.push_back(0);
		std::vector<int> VolumeConstraintsGroupTemp = vSimulationModel.getVolumeConstraintsGroupOffset();
		m_VolumeConstraintOffset.insert(m_VolumeConstraintOffset.end(), VolumeConstraintsGroupTemp.begin(), VolumeConstraintsGroupTemp.end());
		Size = m_VolumeConstraintCount * sizeof(CVolumeConstraint);
		HANDLE_ERROR(cudaMalloc((void**)&m_dVolumeConstraints, Size));
		HANDLE_ERROR(cudaMemcpy(m_dVolumeConstraints, VolumeConstraints.data(), Size, cudaMemcpyHostToDevice));
	}
	
	std::vector<CProjectionConstraint> VolumeProjectionConstraints;
	vSimulationModel.dumpVolumeProjectionConstraints(VolumeProjectionConstraints);
	m_VolumeProjectionConstraintCount = VolumeProjectionConstraints.size();
	if (m_VolumeProjectionConstraintCount > 0)
	{
		Size = m_VolumeProjectionConstraintCount * sizeof(CProjectionConstraint);
		HANDLE_ERROR(cudaMalloc((void**)&m_dVolumeProjectionConstraints, Size));
		HANDLE_ERROR(cudaMemcpy(m_dVolumeProjectionConstraints, VolumeProjectionConstraints.data(), Size, cudaMemcpyHostToDevice));
	}
	
	std::vector<CShapeMatchingConstraint> ShapeMatchingConstraints;
	vSimulationModel.dumpShapeMatchingConstraints(ShapeMatchingConstraints);
	m_ShapeMatchingConstraintCount = ShapeMatchingConstraints.size();
	std::vector<int> particleRegionCount = vSimulationModel.getParticleRelatedClusterCount();
	if (m_ShapeMatchingConstraintCount > 0)
	{
		m_ShapeMatchingConstraintOffset.push_back(0);
		std::vector<int> ShapeMatchingConstraintsGroupTemp = vSimulationModel.getShapeMatchingConstraintsGroupOffset();
		m_ShapeMatchingConstraintOffset.insert(m_ShapeMatchingConstraintOffset.end(), ShapeMatchingConstraintsGroupTemp.begin(), ShapeMatchingConstraintsGroupTemp.end());
		Size = m_ShapeMatchingConstraintCount * sizeof(CShapeMatchingConstraint);
		HANDLE_ERROR(cudaMalloc((void**)&m_dShapeMatchingConstraints, Size));
		HANDLE_ERROR(cudaMemcpy(m_dShapeMatchingConstraints, ShapeMatchingConstraints.data(), Size, cudaMemcpyHostToDevice));

		Size = particleRegionCount.size() * sizeof(int);
		HANDLE_ERROR(cudaMalloc((void**)&m_dParticleRegionCount, Size));
		HANDLE_ERROR(cudaMemcpy(m_dParticleRegionCount, particleRegionCount.data(), Size, cudaMemcpyHostToDevice));
	}
	
	std::vector<CProjectionConstraint> ShapeMatchingProjectionConstraints;
	vSimulationModel.dumpShapeMatchingProjectionConstraints(ShapeMatchingProjectionConstraints);
	m_ShapeMatchingProjectionConstraintCount = ShapeMatchingProjectionConstraints.size();
	if (m_ShapeMatchingProjectionConstraintCount > 0)
	{
		Size = m_ShapeMatchingProjectionConstraintCount * sizeof(CProjectionConstraint);
		HANDLE_ERROR(cudaMalloc((void**)&m_dShapeMatchingProjectionConstraints, Size));
		HANDLE_ERROR(cudaMemcpy(m_dShapeMatchingProjectionConstraints, ShapeMatchingProjectionConstraints.data(), Size, cudaMemcpyHostToDevice));
	}	
}

//*********************************************************************
//FUNCTION:
__host__ bool Animation::CTimeStepController::step()
{
	int BlockSize = std::min(m_MaxThreadsPerBlock, m_ParticleCount);
	int GridSize = (m_ParticleCount + BlockSize - 1) / BlockSize;
	predictPosition<<<GridSize, BlockSize>>>(m_dParticleData, m_TimeStep, m_ParticleCount);
	HANDLE_ERROR(cudaGetLastError());
	HANDLE_ERROR(cudaDeviceSynchronize());

	for (int i = 0; i < m_IterationCount; i++)
	{
		for (int k = 0; k < m_DistanceConstraintOffset.size()-1; k++)
		{
			int GroupSize = m_DistanceConstraintOffset[k + 1] - m_DistanceConstraintOffset[k];
			BlockSize = std::min(m_MaxThreadsPerBlock, GroupSize);
			GridSize = (GroupSize + BlockSize -1) / BlockSize;
			solveDistanceConstraints<<<GridSize, BlockSize>>>(m_dParticleData, m_dDistanceConstraints, m_DistanceConstraintOffset[k + 1], m_DistanceConstraintOffset[k]);
			HANDLE_ERROR(cudaGetLastError());
		}

		if (m_DistanceProjectionConstraintCount > 0)
		{
			BlockSize = std::min(m_MaxThreadsPerBlock, m_DistanceProjectionConstraintCount);
			GridSize = (m_DistanceProjectionConstraintCount + BlockSize - 1) / BlockSize;
			projectionCostraints <<<GridSize, BlockSize >>> (m_dParticleData, m_dDistanceProjectionConstraints, m_DistanceProjectionConstraintCount);
			HANDLE_ERROR(cudaGetLastError());
			//HANDLE_ERROR(cudaDeviceSynchronize());
		}		

		for (int k = 0; k < m_VolumeConstraintOffset.size()-1; k++)
		{
			int GroupSize = m_VolumeConstraintOffset[k + 1] - m_VolumeConstraintOffset[k];
			BlockSize = std::min(m_MaxThreadsPerBlock / 2, GroupSize);
			GridSize = (GroupSize + BlockSize - 1) / BlockSize;
			solveVolumeConstraints<<<GridSize, BlockSize>>>(m_dParticleData, m_dVolumeConstraints, m_VolumeConstraintOffset[k + 1], m_VolumeConstraintOffset[k]);
			HANDLE_ERROR(cudaGetLastError());
		}

		if (m_VolumeProjectionConstraintCount > 0)
		{
			BlockSize = std::min(m_MaxThreadsPerBlock / 2, m_VolumeProjectionConstraintCount);
			GridSize = (m_VolumeProjectionConstraintCount + BlockSize - 1) / BlockSize;
			projectionCostraints <<<GridSize, BlockSize >>> (m_dParticleData, m_dVolumeProjectionConstraints, m_VolumeProjectionConstraintCount);
			HANDLE_ERROR(cudaGetLastError());
			//HANDLE_ERROR(cudaDeviceSynchronize());
		}

		for (int k = 0; k < m_ShapeMatchingConstraintOffset.size()-1; k++)
		{
			int GroupSize = m_ShapeMatchingConstraintOffset[k + 1] - m_ShapeMatchingConstraintOffset[k];
			BlockSize = std::min(m_MaxThreadsPerBlock / 2, GroupSize);
			GridSize = (GroupSize + BlockSize - 1) / BlockSize;
			solveShapeMatchingConstraints<<<GridSize, BlockSize >>>(m_dParticleData, m_dShapeMatchingConstraints, m_dParticleRegionCount, m_ShapeMatchingConstraintOffset[k + 1], m_ShapeMatchingConstraintOffset[k]);
			HANDLE_ERROR(cudaGetLastError());
		}

		if (m_ShapeMatchingProjectionConstraintCount > 0)
		{
			BlockSize = std::min(m_MaxThreadsPerBlock / 2, m_ShapeMatchingProjectionConstraintCount);
			GridSize = (m_ShapeMatchingProjectionConstraintCount + BlockSize - 1) / BlockSize;
			projectionCostraints <<<GridSize, BlockSize >>> (m_dParticleData, m_dShapeMatchingProjectionConstraints, m_ShapeMatchingProjectionConstraintCount);
			HANDLE_ERROR(cudaGetLastError());
			HANDLE_ERROR(cudaDeviceSynchronize());
		}
	}

	BlockSize = std::min(m_MaxThreadsPerBlock / 2, m_OriginParticleCount);
	GridSize = (m_OriginParticleCount + BlockSize - 1) / BlockSize;
	updateParticlePosition<<<GridSize, BlockSize>>>(m_dParticleData, dParticlePos, m_OriginParticleCount);
	HANDLE_ERROR(cudaGetLastError());

	BlockSize = std::min(m_MaxThreadsPerBlock / 2, m_ParticleCount);
	GridSize = (m_ParticleCount + BlockSize - 1) / BlockSize;
	updateVelocity<<<GridSize, BlockSize>>>(m_dParticleData, m_TimeStep, m_DampFactor, m_ParticleCount);
	HANDLE_ERROR(cudaGetLastError());
	HANDLE_ERROR(cudaDeviceSynchronize());

	return true;
}

//*********************************************************************
//FUNCTION:
void Animation::CTimeStepController::updateBoneParticlesWithCuda(const std::vector<int>& vIndices, const std::vector<SVector3d>& vPos)
{
	const int BoneParticleCount = vPos.size();
	const int Size = BoneParticleCount * sizeof(SVector3d);
	const int Indice_Size = BoneParticleCount * sizeof(int);
	_ASSERTE(BoneParticleCount == vIndices.size());
	if (dBoneIndices == nullptr)
	{
		HANDLE_ERROR(cudaMalloc((void**)&dBoneIndices, Indice_Size));
	}
	if (dBonePos == nullptr)
	{
		HANDLE_ERROR(cudaMalloc((void**)&dBonePos, Size));
	}

	HANDLE_ERROR(cudaMemcpy(dBoneIndices, vIndices.data(), Indice_Size, cudaMemcpyHostToDevice));
	HANDLE_ERROR(cudaMemcpy(dBonePos, vPos.data(), Size, cudaMemcpyHostToDevice));

	int BlockSize = std::min(m_MaxThreadsPerBlock, BoneParticleCount);
	int GridSize = (BoneParticleCount + BlockSize - 1) / BlockSize;
	updateBoneParticles<<<GridSize, BlockSize>>>(m_dParticleData, dBoneIndices, dBonePos, BoneParticleCount);
	HANDLE_ERROR(cudaGetLastError());
	HANDLE_ERROR(cudaDeviceSynchronize());
}

//*********************************************************************
//FUNCTION:
std::vector<Eigen::Vector3d> Animation::CTimeStepController::getPositions()
{
	std::vector<Eigen::Vector3d> Positions;

	//HANDLE_ERROR(cudaDeviceSynchronize());
	std::vector<SVector3d> TempParticlePosition(m_OriginParticleCount);
	HANDLE_ERROR(cudaMemcpy(TempParticlePosition.data(), dParticlePos, m_OriginParticleCount * sizeof(SVector3d), cudaMemcpyDeviceToHost));

	Positions.resize(m_OriginParticleCount);
	for (int i = 0; i < m_OriginParticleCount; i++)
	{
		Eigen::Vector3d Position(TempParticlePosition[i].x, TempParticlePosition[i].y, TempParticlePosition[i].z);
		Positions[i] = Position;
	}

	return Positions;
}

//*********************************************************************
//FUNCTION:
void Animation::CTimeStepController::resetParticleData(CSimulationModel& vSimulationModel)
{
	if (m_dParticleData == nullptr)
		return;
	std::vector<CParticleData> ParticleData;
	vSimulationModel.dumpSimulationParticle(ParticleData);
	int Size = m_ParticleCount * sizeof(CParticleData);
	HANDLE_ERROR(cudaMemcpy(m_dParticleData, ParticleData.data(), Size, cudaMemcpyHostToDevice));
	HANDLE_ERROR(cudaDeviceSynchronize());
}

//*********************************************************************
//FUNCTION:
void Animation::CTimeStepController::__freeCudaMemory()
{
	if (nullptr != m_dShapeMatchingConstraints)
		cudaFree(m_dShapeMatchingConstraints);
	
	if(nullptr != m_dShapeMatchingProjectionConstraints)
		cudaFree(m_dShapeMatchingProjectionConstraints);
	
	if(nullptr != m_dVolumeConstraints)
		cudaFree(m_dVolumeConstraints);

	if(nullptr != m_dVolumeProjectionConstraints)
		cudaFree(m_dVolumeProjectionConstraints);

	if(nullptr != m_dDistanceConstraints)
		cudaFree(m_dDistanceConstraints);
	
	if(nullptr != m_dDistanceProjectionConstraints)
		cudaFree(m_dDistanceProjectionConstraints);
	
	if(nullptr != m_dParticleData)
		cudaFree(m_dParticleData);
	
	if(nullptr != dBoneIndices)
		cudaFree(dBoneIndices);
	
	if(nullptr != dBonePos)
		cudaFree(dBonePos);
	
	if(nullptr != dParticlePos)
		cudaFree(dParticlePos);

	if (nullptr != m_dParticleRegionCount)
		cudaFree(m_dParticleRegionCount);
}