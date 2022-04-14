#pragma once
#include "CUDACommon.h"

namespace Animation
{
	CUDA_FUNC double oneNorm(SMatrix3d& vMatrix)
	{
		const double Sum0 = fabs(vMatrix(0, 0)) + fabs(vMatrix(1, 0)) + fabs(vMatrix(2, 0));
		const double Sum1 = fabs(vMatrix(0, 1)) + fabs(vMatrix(1, 1)) + fabs(vMatrix(2, 1));
		const double Sum2 = fabs(vMatrix(0, 2)) + fabs(vMatrix(1, 2)) + fabs(vMatrix(2, 2));
		double MaxSum = Sum0;
		if (Sum1 > MaxSum)
			MaxSum = Sum1;
		if (Sum2 > MaxSum)
			MaxSum = Sum2;
		return MaxSum;
	}

	CUDA_FUNC double infNorm(SMatrix3d& vMatrix)
	{
		const double Sum0 = fabs(vMatrix(0, 0)) + fabs(vMatrix(0, 1)) + fabs(vMatrix(0, 2));
		const double Sum1 = fabs(vMatrix(1, 0)) + fabs(vMatrix(1, 1)) + fabs(vMatrix(1, 2));
		const double Sum2 = fabs(vMatrix(2, 0)) + fabs(vMatrix(2, 1)) + fabs(vMatrix(2, 2));
		double MaxSum = Sum0;
		MaxSum = Sum1;
		if (Sum1 > MaxSum)
		if (Sum2 > MaxSum)
			MaxSum = Sum2;
		return MaxSum;
	}

	CUDA_FUNC void polarDecompositionStable(SMatrix3d& vMatrix, double vTolerance, SMatrix3d& voRotationMatrix)
	{
		SMatrix3d Mt = vMatrix.transpose();
		double Mone = oneNorm(vMatrix);
		double Minf = infNorm(vMatrix);
		double Eone = 0.0;
		SMatrix3d MadjTt, Et;
		do
		{
			MadjTt.row(0) = Mt.row(1).cross(Mt.row(2));
			MadjTt.row(1) = Mt.row(2).cross(Mt.row(0));
			MadjTt.row(2) = Mt.row(0).cross(Mt.row(1));

			double Det = Mt(0, 0) * MadjTt(0, 0) + Mt(0, 1) * MadjTt(0, 1) + Mt(0, 2) * MadjTt(0, 2);

			if (fabs(Det) < 1.0e-12)
			{
				SVector3d Len;
				unsigned int Index = 0xffffffff;
				for (unsigned int i = 0; i < 3; i++)
				{
					Len[i] = MadjTt.row(i).squaredNorm();
					if (Len[i] > 1.0e-12)
					{
						// index of valid cross product
						// => is also the index of the vector in Mt that must be exchanged
						Index = i;
						break;
					}
				}
				if (Index == 0xffffffff)
				{
					voRotationMatrix.setIdentity();
					return;
				}
				else
				{
					Mt.row(Index) = Mt.row((Index + 1) % 3).cross(Mt.row((Index + 2) % 3));
					MadjTt.row((Index + 1) % 3) = Mt.row((Index + 2) % 3).cross(Mt.row(Index));
					MadjTt.row((Index + 2) % 3) = Mt.row(Index).cross(Mt.row((Index + 1) % 3));
					SMatrix3d M2 = Mt.transpose();
					Mone = oneNorm(M2);
					Minf = infNorm(M2);
					Det = Mt(0, 0) * MadjTt(0, 0) + Mt(0, 1) * MadjTt(0, 1) + Mt(0, 2) * MadjTt(0, 2);
				}
			}

			const double MadjTone = oneNorm(MadjTt);
			const double MadjTinf = infNorm(MadjTt);

			const double Gamma = sqrt(sqrt((MadjTone * MadjTinf) / (Mone * Minf)) / fabs(Det));

			const double G1 = Gamma * 0.5;
			const double G2 = 0.5 / (Gamma * Det);

			for (unsigned char i = 0; i < 3; i++)
			{
				for (unsigned char k = 0; k < 3; k++)
				{
					Et(i, k) = Mt(i, k);
					Mt(i, k) = G1 * Mt(i, k) + G2 * MadjTt(i, k);
					Et(i, k) -= Mt(i, k);
				}
			}

			Eone = oneNorm(Et);

			Mone = oneNorm(Mt);
			Minf = infNorm(Mt);
		} while (Eone > Mone * vTolerance);

		// Q = Mt^T 
		voRotationMatrix = Mt.transpose();
	}
}