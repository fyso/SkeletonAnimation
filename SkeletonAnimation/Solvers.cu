#pragma once
#include "CommonMath.h"

namespace Animation
{
	__device__ bool solveDistanceConstraint(const SVector3d &vPos0, double vInvMass0, const SVector3d &vPos1, double vInvMass1,
		const double vRestLength, const double vStiffness, SVector3d &voCorrect0, SVector3d &voCorrect1)
	{
		double WeightSum = vInvMass0 + vInvMass1;
		if (WeightSum < DBL_EPSILON)
			return false;

		SVector3d Vector = vPos1 - vPos0;
		double Distance = Vector.norm();
		Vector.normalize();

		SVector3d Correct = Vector * vStiffness * (Distance - vRestLength) / WeightSum;

		voCorrect0 = Correct * vInvMass0;
		voCorrect1 = Correct * -1.0 * vInvMass1;
		return true;
	}

	__device__ bool solveVolumeConstraint(const SVector3d &vPos0, double vInvMass0, const SVector3d &vPos1, double vInvMass1,
		const SVector3d &vPos2, double vInvMass2, const SVector3d &vPos3, double vInvMass3,
		const double vRestVolume, const double vNegVolumeStiffness, const double vPosVolumeStiffness,
		SVector3d &voCorrect0, SVector3d &voCorrect1, SVector3d &voCorrect2, SVector3d &voCorrect3)
	{
		double Volume = 1.0 / 6.0 * (vPos1 - vPos0).cross(vPos2 - vPos0).dot(vPos3 - vPos0);

		voCorrect0.setZero(); voCorrect1.setZero(); voCorrect2.setZero(); voCorrect3.setZero();

		if (vPosVolumeStiffness == 0.0 && Volume > 0.0)
			return false;

		if (vNegVolumeStiffness == 0.0 && Volume < 0.0)
			return false;

		SVector3d Grad0 = (vPos1 - vPos2).cross(vPos3 - vPos2);
		SVector3d Grad1 = (vPos2 - vPos0).cross(vPos3 - vPos0);
		SVector3d Grad2 = (vPos0 - vPos1).cross(vPos3 - vPos1);
		SVector3d Grad3 = (vPos1 - vPos0).cross(vPos2 - vPos0);

		double Lambda =
			vInvMass0 * Grad0.squaredNorm() +
			vInvMass1 * Grad1.squaredNorm() +
			vInvMass2 * Grad2.squaredNorm() +
			vInvMass3 * Grad3.squaredNorm();

		if (fabs(Lambda) < DBL_EPSILON)
			return false;

		if (Volume < 0.0)
			Lambda = vNegVolumeStiffness * (Volume - vRestVolume) / Lambda;
		else
			Lambda = vPosVolumeStiffness * (Volume - vRestVolume) / Lambda;

		voCorrect0 = Grad0 * -1.0 * Lambda * vInvMass0;
		voCorrect1 = Grad1 * -1.0 * Lambda * vInvMass1;
		voCorrect2 = Grad2 * -1.0 * Lambda * vInvMass2;
		voCorrect3 = Grad3 * -1.0 * Lambda * vInvMass3;

		return true;
	}

	__device__ bool solveShapeMathingConstrint(
		const SVector3d vX0[], const SVector3d vX[], const double vInvMasses[], const int vPointCount,
		const SVector3d &vRestCm,
		const SMatrix3d &vInvRestMat,
		const double vStiffness[],
		SVector3d voCorrect[])
	{
		for (int i = 0; i < vPointCount; i++)
			voCorrect[i].setZero();

		// center of mass
		SVector3d Cm(0.0);
		double WeightSum = 0.0;
		for (int i = 0; i < vPointCount; i++)
		{
			double Weight = 1.0 / (vInvMasses[i] + DBL_EPSILON);
			Cm += vX[i] * Weight;
			WeightSum += Weight;
		}
		if (WeightSum == 0.0)
			return false;
		Cm = Cm / WeightSum;

		SMatrix3d Matrix;
		Matrix.setZero();
		for (int i = 0; i < vPointCount; i++) {
			SVector3d Q = vX0[i] - vRestCm;
			SVector3d P = vX[i] - Cm;

			double Weight = 1.0 / (vInvMasses[i] + DBL_EPSILON);
			P = P * Weight;

			Matrix(0, 0) += P[0] * Q[0]; Matrix(0, 1) += P[0] * Q[1]; Matrix(0, 2) += P[0] * Q[2];
			Matrix(1, 0) += P[1] * Q[0]; Matrix(1, 1) += P[1] * Q[1]; Matrix(1, 2) += P[1] * Q[2];
			Matrix(2, 0) += P[2] * Q[0]; Matrix(2, 1) += P[2] * Q[1]; Matrix(2, 2) += P[2] * Q[2];
		}

		Matrix = Matrix * vInvRestMat;// 具有旋转分量的变换矩阵mat

		SMatrix3d R = Matrix;
		polarDecompositionStable(Matrix, 1e-6, R);

		for (int i = 0; i < vPointCount; i++) {
			SVector3d Goal = Cm + R * (vX0[i] - vRestCm);
			voCorrect[i] = (Goal - vX[i]) * vStiffness[i];
		}

		return true;
	}
}