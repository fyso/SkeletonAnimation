#pragma once
#include <device_launch_parameters.h>
#include <cuda_runtime.h>
#include <vector>
#include "Common.h"

namespace Animation
{
#define DBL_EPSILON      2.2204460492503131e-016 // smallest such that 1.0+DBL_EPSILON != 1.0
#define CUDA_FUNC __host__ __device__

#define HANDLE_ERROR( err ) ( HandleError( err, __FILE__, __LINE__ ) )
	static void HandleError(cudaError_t vErr, const char *vFile, int vLine)
	{
		if (vErr != cudaSuccess)
		{
			fprintf(stderr, "%s in %s at line %d\n", cudaGetErrorString(vErr),
				vFile, vLine);
			system("pause");
			exit(EXIT_FAILURE);
		}
	}

	struct SVector3d
	{
		double x = 0.0;
		double y = 0.0;
		double z = 0.0;

		CUDA_FUNC SVector3d() = default;
		CUDA_FUNC SVector3d(double vElement) : x(vElement), y(vElement), z(vElement) {}
		CUDA_FUNC SVector3d(double vX, double vY, double vZ) : x(vX), y(vY), z(vZ) {}

		CUDA_FUNC double norm() const { return sqrt(x * x + y * y + z * z); }

		CUDA_FUNC double squaredNorm() const { return x * x + y * y + z * z; }

		CUDA_FUNC void setZero()
		{
			x = 0.0;
			y = 0.0;
			z = 0.0;
		}

		CUDA_FUNC SVector3d cross(const SVector3d& vVector) const
		{
			SVector3d Result;

			Result.x = y * vVector.z - vVector.y * z;
			Result.y = vVector.x * z - x * vVector.z;
			Result.z = x * vVector.y - vVector.x * y;

			return Result;
		}

		CUDA_FUNC double dot(const SVector3d& vVector) const { return x * vVector.x + y * vVector.y + z * vVector.z; }

		CUDA_FUNC void normalize()
		{
			double Length = norm();
			x = x / Length;
			y = y / Length;
			z = z / Length;
		}

		CUDA_FUNC double& operator [](int vIndex)
		{
			switch (vIndex)
			{
			case 0:
				return x;
			case 1:
				return y;
			case 2:
				return z;
			default:
				break;
			}
		}

		CUDA_FUNC double& operator ()(int vIndex)
		{
			switch (vIndex)
			{
			case 0:
				return x;
			case 1:
				return y;
			case 2:
				return z;
			default:
				break;
			}
		}

		CUDA_FUNC void operator =(const SVector3d& vOperand)
		{
			x = vOperand.x;
			y = vOperand.y;
			z = vOperand.z;
		}

		CUDA_FUNC void operator +=(const SVector3d& vOperand)
		{
			x = x + vOperand.x;
			y = y + vOperand.y;
			z = z + vOperand.z;
		}

		CUDA_FUNC void operator -=(const SVector3d& vOperand)
		{
			x = x - vOperand.x;
			y = y - vOperand.y;
			z = z - vOperand.z;
		}

		CUDA_FUNC SVector3d operator +(const SVector3d& vOperand) const
		{
			return SVector3d(x + vOperand.x, y + vOperand.y, z + vOperand.z);
		}

		CUDA_FUNC SVector3d operator -(const SVector3d& vOperand) const
		{
			return SVector3d(x - vOperand.x, y - vOperand.y, z - vOperand.z);
		}

		CUDA_FUNC SVector3d operator *(const SVector3d& vOperand) const
		{
			return SVector3d(x * vOperand.x, y * vOperand.y, z * vOperand.z);
		}

		CUDA_FUNC SVector3d operator /(const SVector3d& vOperand) const
		{
			return SVector3d(x / vOperand.x, y / vOperand.y, z / vOperand.z);
		}

		template <typename Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			ar & x;
			ar & y;
			ar & z;
		}

	};

	struct SMatrix3d
	{
	private:
		SVector3d row0;
		SVector3d row1;
		SVector3d row2;

	public:
		CUDA_FUNC void setIdentity()
		{
			row0.x = 1.0;
			row1.y = 1.0;
			row2.z = 1.0;
		}

		CUDA_FUNC void setZero()
		{
			row0.setZero();
			row1.setZero();
			row2.setZero();
		}

		CUDA_FUNC void operator =(const SMatrix3d& vOperand)
		{
			row0 = vOperand.row0;
			row1 = vOperand.row1;
			row2 = vOperand.row2;
		}

		CUDA_FUNC void operator +=(const SMatrix3d& vOperand)
		{
			row0 += vOperand.row0;
			row1 += vOperand.row1;
			row2 += vOperand.row2;
		}

		CUDA_FUNC double& operator ()(int vRow, int vCol)
		{
			switch (vRow)
			{
			case 0:
				return row0(vCol);
			case 1:
				return row1(vCol);
			case 2:
				return row2(vCol);
			default:
				break;
			}
		}

		CUDA_FUNC SMatrix3d transpose() const
		{
			SMatrix3d Result;

			Result.row0.x = row0.x;
			Result.row0.y = row1.x;
			Result.row0.z = row2.x;
			Result.row1.x = row0.y;
			Result.row1.y = row1.y;
			Result.row1.z = row2.y;
			Result.row2.x = row0.z;
			Result.row2.y = row1.z;
			Result.row2.z = row2.z;

			return Result;
		}

		CUDA_FUNC SVector3d& row(int vIndex)
		{
			switch (vIndex)
			{
			case 0:
				return row0;
			case 1:
				return row1;
			case 2:
				return row2;
			default:
				break;
			}
		}

		CUDA_FUNC SVector3d col(int vIndex) const
		{
			SVector3d Result;

			switch (vIndex)
			{
			case 0:
				Result.x = row0.x;
				Result.y = row1.x;
				Result.z = row2.x;
				break;
			case 1:
				Result.x = row0.y;
				Result.y = row1.y;
				Result.z = row2.y;
				break;
			case 2:
				Result.x = row0.z;
				Result.y = row1.z;
				Result.z = row2.z;
				break;
			default:
				break;
			}

			return Result;
		}

		CUDA_FUNC void setCol(int vIndex, SVector3d vVector)
		{
			switch (vIndex)
			{
			case 0:
				row0.x = vVector.x;
				row1.x = vVector.y;
				row2.x = vVector.z;
				break;
			case 1:
				row0.y = vVector.x;
				row1.y = vVector.y;
				row2.y = vVector.z;
				break;
			case 2:
				row0.z = vVector.x;
				row1.z = vVector.y;
				row2.z = vVector.z;
				break;
			default:
				break;
			}
		}

		CUDA_FUNC SMatrix3d operator *(const SMatrix3d& vOperand) const
		{
			SMatrix3d Result;

			Result.row0.x = row0.x*vOperand.row0.x + row0.y*vOperand.row1.x + row0.z*vOperand.row2.x;
			Result.row0.y = row0.x*vOperand.row0.y + row0.y*vOperand.row1.y + row0.z*vOperand.row2.y;
			Result.row0.z = row0.x*vOperand.row0.z + row0.y*vOperand.row1.z + row0.z*vOperand.row2.z;
			Result.row1.x = row1.x*vOperand.row0.x + row1.y*vOperand.row1.x + row1.z*vOperand.row2.x;
			Result.row1.y = row1.x*vOperand.row0.y + row1.y*vOperand.row1.y + row1.z*vOperand.row2.y;
			Result.row1.z = row1.x*vOperand.row0.z + row1.y*vOperand.row1.z + row1.z*vOperand.row2.z;
			Result.row2.x = row2.x*vOperand.row0.x + row2.y*vOperand.row1.x + row2.z*vOperand.row2.x;
			Result.row2.y = row2.x*vOperand.row0.y + row2.y*vOperand.row1.y + row2.z*vOperand.row2.y;
			Result.row2.z = row2.x*vOperand.row0.z + row2.y*vOperand.row1.z + row2.z*vOperand.row2.z;

			return Result;
		}

		CUDA_FUNC SVector3d operator *(const SVector3d& vOperand) const
		{
			SVector3d Result;

			Result.x = row0.x*vOperand.x + row0.y*vOperand.y + row0.z*vOperand.z;
			Result.y = row1.x*vOperand.x + row1.y*vOperand.y + row1.z*vOperand.z;
			Result.z = row2.x*vOperand.x + row2.y*vOperand.y + row2.z*vOperand.z;

			return Result;
		}

		template <typename Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			ar & row0;
			ar & row1;
			ar & row2;
		}

	};
}