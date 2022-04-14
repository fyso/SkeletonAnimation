#pragma once
#include "DistanceConstraint.h"
#include "VolumeConstraint.h"
#include "ShapeMatchingConstraint.h"
#include "ProjectionConstraint.h"
#include <stdlib.h>
#include <stdio.h>
#include "Solvers.cu"

namespace Animation
{
	__global__ void updateBoneParticles(CParticleData* vioParticleData, int* vIndices, SVector3d* vPos, int vSize)
	{
		int Index = blockIdx.x * blockDim.x + threadIdx.x;
		if (Index >= vSize) return;

		vioParticleData[vIndices[Index]].setPosition(vPos[Index]);
	}

	__global__ void predictPosition(CParticleData* vioParticleData, double vTimeStep, int vSize)
	{
		int Index = blockIdx.x * blockDim.x + threadIdx.x;
		if (Index >= vSize) return;

		vioParticleData[Index].fetchLastPosition() = vioParticleData[Index].getOldPosition();
		vioParticleData[Index].fetchOldPosition() = vioParticleData[Index].getPosition();

		if (vioParticleData[Index].getMass() > DBL_EPSILON)
		{
			vioParticleData[Index].fetchVelocity() += vioParticleData[Index].getAcceleration() * vTimeStep;
			vioParticleData[Index].fetchPosition() += vioParticleData[Index].getVelocity() * vTimeStep;
		}
	}

	__global__ void solveDistanceConstraints(CParticleData* vioParticleData, CDistanceConstraint* vDistanceConstraints, int vSize, int vOffset)
	{
		int Index = vOffset + blockIdx.x * blockDim.x + threadIdx.x;
		if (Index >= vSize) return;

		CDistanceConstraint DistanceConstraint = vDistanceConstraints[Index];
		int* ParticleIndices = DistanceConstraint.getParticleIndices();
		unsigned int Index0 = ParticleIndices[0];
		unsigned int Index1 = ParticleIndices[1];

		SVector3d X0 = vioParticleData[Index0].getPosition();
		SVector3d X1 = vioParticleData[Index1].getPosition();
		double InvMass0 = vioParticleData[Index0].getInverseMass();
		double InvMass1 = vioParticleData[Index1].getInverseMass();

		double RestDistance = DistanceConstraint.getRestDistance();
		double Stiffness = DistanceConstraint.getStiffness();
		SVector3d Correct0, Correct1;
		bool Result = solveDistanceConstraint(X0, InvMass0, X1, InvMass1, RestDistance, Stiffness, Correct0, Correct1);
		if (Result)
		{
			if (InvMass0 > DBL_EPSILON)
				vioParticleData[Index0].fetchPosition() += Correct0;
			if (InvMass1 > DBL_EPSILON)
				vioParticleData[Index1].fetchPosition() += Correct1;
		}
	}

	__global__ void solveVolumeConstraints(CParticleData* vioParticleData, CVolumeConstraint* vVolumeConstraints, int vSize, int vOffset)
	{
		int Index = vOffset + blockIdx.x * blockDim.x + threadIdx.x;
		if (Index >= vSize) return;

		CVolumeConstraint VolumeConstraint = vVolumeConstraints[Index];
		int* ParticleIndices = VolumeConstraint.getParticleIndices();
		unsigned int Index0 = ParticleIndices[0];
		unsigned int Index1 = ParticleIndices[1];
		unsigned int Index2 = ParticleIndices[2];
		unsigned int Index3 = ParticleIndices[3];

		SVector3d X0 = vioParticleData[Index0].getPosition();
		SVector3d X1 = vioParticleData[Index1].getPosition();
		SVector3d X2 = vioParticleData[Index2].getPosition();
		SVector3d X3 = vioParticleData[Index3].getPosition();

		double InvMass0 = vioParticleData[Index0].getInverseMass();
		double InvMass1 = vioParticleData[Index1].getInverseMass();
		double InvMass2 = vioParticleData[Index2].getInverseMass();
		double InvMass3 = vioParticleData[Index3].getInverseMass();

		double RestVolume = VolumeConstraint.getRestVolume();
		double Stiffness = VolumeConstraint.getStiffness();
		SVector3d Correct0, Correct1, Correct2, Correct3;
		bool Result = solveVolumeConstraint(X0, InvMass0, X1, InvMass1, X2, InvMass2, X3, InvMass3, RestVolume, Stiffness, Stiffness, Correct0, Correct1, Correct2, Correct3);
		if (Result)
		{
			if (InvMass0 > DBL_EPSILON)
				vioParticleData[Index0].fetchPosition() += Correct0;
			if (InvMass1 > DBL_EPSILON)
				vioParticleData[Index1].fetchPosition() += Correct1;
			if (InvMass2 > DBL_EPSILON)
				vioParticleData[Index2].fetchPosition() += Correct2;
			if (InvMass3 > DBL_EPSILON)
				vioParticleData[Index3].fetchPosition() += Correct3;
		}
	}

	__global__ void solveShapeMatchingConstraints(CParticleData* vioParticleData, CShapeMatchingConstraint* vShapeMatchingConstraints, int* vParticleRegionCount, int vSize, int vOffset)
	{
		int Index = vOffset + blockIdx.x * blockDim.x + threadIdx.x;
		if (Index >= vSize) return;

		CShapeMatchingConstraint ShapeMatchingConstraint = vShapeMatchingConstraints[Index];
		int ParticleCount = ShapeMatchingConstraint.getParticleCount();
		int* ParticleIndices = ShapeMatchingConstraint.getParticleIndices();
		SVector3d CurX[27];
		for (int i = 0; i < ParticleCount; i++)
		{
			CurX[i] = vioParticleData[ParticleIndices[i]].getPosition();
		}

		SVector3d* X0 = ShapeMatchingConstraint.getPosition0();
		double* Weight = ShapeMatchingConstraint.getWeight();
		SVector3d RestCm = ShapeMatchingConstraint.getRestCm();
		SMatrix3d InvRestMat = ShapeMatchingConstraint.getInvRestMat();
		double* Stiffness = ShapeMatchingConstraint.getStiffness();
		SVector3d Correct[27];
		bool Result = solveShapeMathingConstrint(X0, CurX, Weight, ParticleCount, RestCm, InvRestMat, Stiffness, Correct);
		if (Result)
		{
			for (unsigned int i = 0; i < ParticleCount; i++)
			{
				// Important: Divide position correction by the number of clusters which contain the vertex. 
				if (Weight[i] > DBL_EPSILON)
				{
					const int ParticleIndex = ParticleIndices[i];
					int ClusterCount = vParticleRegionCount[ParticleIndex];
					SVector3d DeltaX = Correct[i] * (1.0 / ClusterCount);
					vioParticleData[ParticleIndex].fetchPosition() += DeltaX;
				}
			}
		}
	}

	__global__ void projectionCostraints(CParticleData* vioParticleData, CProjectionConstraint* vProjectionConstraints, int vSize)
	{
		int Index = blockIdx.x * blockDim.x + threadIdx.x;
		if (Index >= vSize) return;

		CProjectionConstraint ProjectionConstraint = vProjectionConstraints[Index];
		SVector3d Position;
		const int Count = ProjectionConstraint.getParticleCount();
		int* ParticleIndices = ProjectionConstraint.getParticleIndices();
		double SumMass = 0.0;
		for (int i = 0; i < Count; i++)
		{
			const int ParticleIndex = ParticleIndices[i];
			double Mass = vioParticleData[ParticleIndex].getMass();
			Position += vioParticleData[ParticleIndex].getPosition() * Mass;
			SumMass += Mass;
		}
		if (SumMass > DBL_EPSILON)
		{
			Position = Position / SumMass;
			for (int i = 0; i < Count; i++)
			{
				const int ParticleIndex = ParticleIndices[i];
				vioParticleData[ParticleIndex].setPosition(Position);
			}
		}
	}

	__global__ void updateParticlePosition(CParticleData* vioParticleData, SVector3d* voParticlePosition, int vSize)
	{
		int Index = blockIdx.x * blockDim.x + threadIdx.x;
		if (Index >= vSize) return;

		voParticlePosition[Index] = vioParticleData[Index].getPosition();
	}

	__global__ void updateVelocity(CParticleData* vioParticleData, double vTimeStep, double vDampFactor, int vSize)
	{
		int Index = blockIdx.x * blockDim.x + threadIdx.x;
		if (Index >= vSize) return;

		if (vioParticleData[Index].getMass() > DBL_EPSILON)
		{
			vioParticleData[Index].fetchVelocity() = (vioParticleData[Index].getPosition() - vioParticleData[Index].getOldPosition()) * (1.0 / vTimeStep);
			vioParticleData[Index].fetchVelocity() = vioParticleData[Index].getVelocity() * (1.0 - vDampFactor);
		}
	}

	__global__ void collide(CParticleData* vioParticleData, int vSize)
	{
		int Index = blockIdx.x * blockDim.x + threadIdx.x;
		if (Index >= vSize) return;

		if (vioParticleData[Index].getPosition().y < 0.0)
		{
			vioParticleData[Index].fetchPosition().y = 0.0;
			vioParticleData[Index].fetchVelocity().y *= -1.0;
		}
	}
}