#include "pch.h"
#include "CommonMath.h"
#include <common/MathInterface.h>
#include <Eigen/Dense>

constexpr double EPSILON = 1e-6;

class CTestDecomposition : public testing::Test
{
protected:
	virtual void SetUp() override
	{
		m_Matrices.resize(m_MatrixCount, Eigen::Matrix3d::Zero());
		for (int i = 0; i < m_MatrixCount; i++)
		{

			const Eigen::Quaterniond Quaterion = Eigen::Quaterniond::UnitRandom();
			Eigen::Matrix3d Matrix = Eigen::Matrix3d::Identity();
			Matrix.row(0) *= hiveMath::hiveGenerateRandomReal<double>(0, 10.0);
			Matrix.row(1) *= hiveMath::hiveGenerateRandomReal<double>(0, 10.0);
			Matrix.row(2) *= hiveMath::hiveGenerateRandomReal<double>(0, 10.0);

			Matrix *= Quaterion.toRotationMatrix();
			m_Matrices[i] = Matrix;
		}
	}

	bool _isEqual(const Eigen::Matrix3d& vLeftMatrix, const Eigen::Matrix3d& vRightMatrix)
	{
		for (int i = 0; i < 3; i++)
		{
			for (int k = 0; k < 3; k++)
			{
				if (fabs(vLeftMatrix(i, k) - vRightMatrix(i, k)) >= EPSILON)
					return false;
			}
		}

		return true;
	}

	bool _isOrthogonal(const Eigen::Matrix3d& vMatrix)
	{
		for (int i = 0; i < 3; i++)
		{
			const Eigen::Vector3d ColVector = vMatrix.col(i);
			const Eigen::Vector3d RowVector = vMatrix.row(i);

			if (fabs(ColVector.norm() - 1.0) >= EPSILON || fabs(RowVector.norm() - 1.0) >= EPSILON)
				return false;
		}

		for (int i = 0; i < 2; i++)
		{
			const Eigen::Vector3d ColVector = vMatrix.col(i);
			const Eigen::Vector3d RowVector = vMatrix.row(i);
			for (int k = i + 1; k < 3; k++)
			{
				const Eigen::Vector3d CurColVector = vMatrix.col(k);
				const Eigen::Vector3d CurRowVector = vMatrix.row(k);

				const float ColRes = ColVector.dot(CurColVector);
				const float RowRes = RowVector.dot(CurRowVector);

				if (fabs(ColRes) >= EPSILON || fabs(RowRes) >= EPSILON)
					return false;
			}
		}

		return true;
	}

	bool _isValidRotationMatrix(const Eigen::Matrix3d& vRotationMatrix)
	{
		// ��ת����������������ת�þ���
		const Eigen::Matrix3d TransposeMatrix = vRotationMatrix.transpose();
		const Eigen::Matrix3d MultiPlyTransposeRes = vRotationMatrix * TransposeMatrix;
		if (!_isEqual(MultiPlyTransposeRes, Eigen::Matrix3d::Identity()))
			return false;

		// ��ת���������ʽΪ1
		if (fabs(vRotationMatrix.determinant() - 1.0) >= EPSILON)
			return false;

		// ��ת��������������
		if (!(_isOrthogonal(vRotationMatrix) && _isOrthogonal(vRotationMatrix.transpose())))
			return false;

		return true;
	}

	Animation::SMatrix3d _convertMatrix(const Eigen::Matrix3d& vMatrix)
	{
		Animation::SMatrix3d Matrix;

		for (int i = 0; i < 3; i++)
		{
			for (int k = 0; k < 3; k++)
			{
				Matrix(i, k) = vMatrix(i, k);
			}
		}

		return Matrix;
	}

	Eigen::Matrix3d _convertEigenMatrix(Animation::SMatrix3d vMatrix)
	{
		Eigen::Matrix3d Matrix;

		for (int i = 0; i < 3; i++)
		{
			for (int k = 0; k < 3; k++)
			{
				Matrix(i, k) = vMatrix(i, k);
			}
		}

		return Matrix;
	}

	int m_MatrixCount = 1000;
	std::vector<Eigen::Matrix3d> m_Matrices;
};

//**************************************
//���Ե㣺�Ӿ����м��ֽ����ת����
//ͨ������:
//	�������ָ����Ŀ�Ĵ���ת�����ľ��󣬷ֽ�õ��ľ���������ת�����Ҫ��
TEST_F(CTestDecomposition, TestPropertiesOfRotationMatrix)
{
	for (int i = 0; i < m_MatrixCount; i++)
	{
		Animation::SMatrix3d Matrix = _convertMatrix(m_Matrices[i]);
		Animation::SMatrix3d RotationMatrix;
		Animation::polarDecompositionStable(Matrix, EPSILON, RotationMatrix);
		Eigen::Matrix3d EigenRotationMatrix = _convertEigenMatrix(RotationMatrix);
		ASSERT_TRUE(_isValidRotationMatrix(EigenRotationMatrix));
	}
}