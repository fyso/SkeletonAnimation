#include "pch.h"
#include "ShapeMatchingConstraint.h"
#include "PositionBasedDynamics.cu"

class CTestPositionBasedDynamics : public testing::Test
{
protected:
	virtual void TearDown() override
	{
		_freeCuda();

		delete m_pShapeMatchingConstraint;
		m_pShapeMatchingConstraint = nullptr;
	}

	void _initShapeMatchingParticles()
	{
		m_ParticleData[0].setInfo({ 0, 0, 0 }, m_Gravity);
		m_ParticleData[1].setInfo({ 0, 1.0, 0 }, m_Gravity);
		m_ParticleData[2].setInfo({ 1.0, 0, 0 }, m_Gravity);
		m_ParticleData[0].setMass(0.0);

		m_pShapeMatchingConstraint = new Animation::CShapeMatchingConstraint(m_ParticleCount);
		const std::vector<int> ParticleIndices = { 0, 1, 2 };
		const std::vector<int> ClusterCount(m_ParticleCount, 1);
		const std::vector<double> Stiffness(m_ParticleCount, 1.0);
		m_pShapeMatchingConstraint->initializeConstraint(m_ParticleData, ParticleIndices, ClusterCount, Stiffness);

		int Size = m_ShapeMatchingConstraintCount * sizeof(Animation::CShapeMatchingConstraint);
		cudaError_t cudaStatus = cudaMalloc((void**)&m_dShapeMatchingConstraints, Size);
		if (cudaStatus != cudaSuccess) ASSERT_TRUE(false);
		cudaStatus = cudaMemcpy(m_dShapeMatchingConstraints, m_pShapeMatchingConstraint, Size, cudaMemcpyHostToDevice);
		if (cudaStatus != cudaSuccess) ASSERT_TRUE(false);
	}

	void _initDistanceParticles()
	{
		m_ParticleData[0].setInfo({ 0, 0, 0 }, m_Gravity);
		m_ParticleData[1].setInfo({ 0, 1.0, 0 }, m_Gravity);
		m_ParticleData[2].setInfo({ 1.0, 1.0, 0 }, m_Gravity);
		m_ParticleData[0].setMass(0.0);

		m_DistanceConstraints.resize(m_DistanceConstraintCount);
		m_DistanceConstraints[0].initializeConstraint(m_ParticleData, { 0, 1 }, 1.0);
		m_DistanceConstraints[1].initializeConstraint(m_ParticleData, { 1, 2 }, 1.0);

		int Size = m_DistanceConstraintCount * sizeof(Animation::CDistanceConstraint);
		cudaError_t cudaStatus = cudaMalloc((void**)&m_dDistanceConstraints, Size);
		if (cudaStatus != cudaSuccess) ASSERT_TRUE(false);
		cudaStatus = cudaMemcpy(m_dDistanceConstraints, m_DistanceConstraints.data(), Size, cudaMemcpyHostToDevice);
		if (cudaStatus != cudaSuccess) ASSERT_TRUE(false);
	}

	void _initVolumeParticles()
	{
		m_ParticleData[0].setInfo({ 0, 0, 0 }, m_Gravity);
		m_ParticleData[1].setInfo({ 0, 1.0, 0 }, m_Gravity);
		m_ParticleData[2].setInfo({ 1.0, 0, 0 }, m_Gravity);
		m_ParticleData[3].setInfo({ 0, 0, 1.0 }, m_Gravity);
		m_ParticleData[0].setMass(0.0);
		
		m_pVolumeConstraints = new Animation::CVolumeConstraint();
		std::vector<int> Indices = { 0, 1, 2, 3 };
		m_pVolumeConstraints->initializeConstraint(m_ParticleData, Indices, 1.0);

		int Size = m_VolumeConstraintCount * sizeof(Animation::CVolumeConstraint);
		cudaError_t cudaStatus = cudaMalloc((void**)&m_dVolumeConstraints, Size);
		if (cudaStatus != cudaSuccess) ASSERT_TRUE(false);
		cudaStatus = cudaMemcpy(m_dVolumeConstraints, m_pVolumeConstraints, Size, cudaMemcpyHostToDevice);
		if (cudaStatus != cudaSuccess) ASSERT_TRUE(false);
	}

	void _calAreaAndAngles(double& voArea, std::vector<double>& voAngles)
	{
		Animation::SVector3d EdgeA = m_ParticleData[1].getPosition() - m_ParticleData[0].getPosition();
		Animation::SVector3d EdgeB = m_ParticleData[2].getPosition() - m_ParticleData[0].getPosition();
		Animation::SVector3d EdgeC = m_ParticleData[2].getPosition() - m_ParticleData[1].getPosition();

		voArea = _calArea(EdgeA.norm(), EdgeB.norm(), EdgeC.norm());

		voAngles.resize(m_ParticleCount);
		EdgeA.normalize();
		EdgeB.normalize();
		EdgeC.normalize();
		voAngles[0] = EdgeA.dot(EdgeB);
		voAngles[1] = EdgeA.dot(EdgeC);
		voAngles[2] = EdgeB.dot(EdgeC);
	}

	inline double _calArea(double vEdgeA, double vEdgeB, double vEdgeC)
	{
		const double P = (vEdgeA + vEdgeB + vEdgeC) / 2;

		return sqrt(P* (P - vEdgeA) * (P - vEdgeB) * (P - vEdgeC));
	}

	void _isEqual(const Animation::SVector3d& vLeft, const Animation::SVector3d& vRight)
	{
		EXPECT_DOUBLE_EQ(vLeft.x, vRight.x);
		EXPECT_DOUBLE_EQ(vLeft.y, vRight.y);
		EXPECT_DOUBLE_EQ(vLeft.z, vRight.z);
	}

	void _isNear(const Animation::SVector3d& vLeft, const Animation::SVector3d& vRight, double vTolerance)
	{
		EXPECT_NEAR(vLeft.x, vRight.x, vTolerance);
		EXPECT_NEAR(vLeft.y, vRight.y, vTolerance);
		EXPECT_NEAR(vLeft.z, vRight.z, vTolerance);
	}

	void _initCuda()
	{
		cudaError_t cudaStatus = cudaSuccess;
		cudaDeviceProp DeviceProp;
		cudaStatus = cudaGetDeviceProperties(&DeviceProp, 0);
		if (cudaStatus != cudaSuccess) ASSERT_TRUE(false);
		m_MaxThreadsPerBlock = DeviceProp.maxThreadsPerBlock;
		cudaStatus = cudaSetDevice(0);
		if (cudaStatus != cudaSuccess) ASSERT_TRUE(false);

		int Size = m_ParticleCount * sizeof(Animation::CParticleData);
		cudaStatus = cudaMalloc((void**)&m_dParticleData, Size);
		if (cudaStatus != cudaSuccess) ASSERT_TRUE(false);
		cudaStatus = cudaMemcpy(m_dParticleData, m_ParticleData.data(), Size, cudaMemcpyHostToDevice);
		if (cudaStatus != cudaSuccess) ASSERT_TRUE(false);

		//std::vector<Animation::CLock> Locks(m_ParticleCount);
		//Size = m_ParticleCount * sizeof(Animation::CLock);
		//cudaStatus = cudaMalloc((void**)&m_dLocks, Size);
		//if (cudaStatus != cudaSuccess) ASSERT_TRUE(false);
		//cudaStatus = cudaMemcpy(m_dLocks, Locks.data(), Size, cudaMemcpyHostToDevice);
		//if (cudaStatus != cudaSuccess) ASSERT_TRUE(false);
	}

	void _freeCuda()
	{
		cudaFree(m_dDistanceConstraints);
		cudaFree(m_dVolumeConstraints);
		cudaFree(m_dShapeMatchingConstraints);
		//cudaFree(m_dLocks);
		cudaFree(m_dParticleData);
	}

	int m_ParticleCount = 0;
	std::vector<Animation::CParticleData> m_ParticleData;
	Animation::CShapeMatchingConstraint* m_pShapeMatchingConstraint = nullptr;
	std::vector<Animation::CDistanceConstraint> m_DistanceConstraints;
	Animation::CVolumeConstraint *m_pVolumeConstraints = nullptr;
	Animation::SVector3d m_Gravity = { 0, 10.0, 0 };

	int m_ShapeMatchingConstraintCount = 1;
	int m_DistanceConstraintCount = 2;
	int m_VolumeConstraintCount = 1;
	int m_MaxThreadsPerBlock = 0;
	Animation::CParticleData *m_dParticleData = nullptr;
	Animation::CLock *m_dLocks = nullptr;
	Animation::CShapeMatchingConstraint *m_dShapeMatchingConstraints = nullptr;
	Animation::CDistanceConstraint *m_dDistanceConstraints = nullptr;
	Animation::CVolumeConstraint *m_dVolumeConstraints = nullptr;
};

//**************************************
//测试点：测试解算形状匹配约束的结果是否正确
//通过条件: 
//		1、粒子的位置在一定范围
//		2、粒子组成的面积/体积相等
//		3、粒子间的夹角角度大致相等
TEST_F(CTestPositionBasedDynamics, TestShapeMatchingConstraint)
{
	m_ParticleCount = 3;
	m_ParticleData.resize(m_ParticleCount);
	_initShapeMatchingParticles();
	double ExpectArea;
	std::vector<double> ExpectAngles;
	_calAreaAndAngles(ExpectArea, ExpectAngles);

	m_ParticleData[0].setPosition({ -1.0, 0, 0 });

	_initCuda();

	Animation::solveShapeMatchingConstraints<<<1, 1>>>(m_dParticleData, m_dShapeMatchingConstraints, m_dLocks, m_ShapeMatchingConstraintCount);
	cudaError_t cudaStatus = cudaGetLastError();
	if (cudaStatus != cudaSuccess) ASSERT_TRUE(false);
	cudaStatus = cudaDeviceSynchronize();
	if (cudaStatus != cudaSuccess) ASSERT_TRUE(false);

	std::vector<Animation::CParticleData> TempParticleData(m_ParticleCount);
	// Copy output vector from GPU buffer to host memory.
	cudaStatus = cudaMemcpy(TempParticleData.data(), m_dParticleData, m_ParticleCount * sizeof(Animation::CParticleData), cudaMemcpyDeviceToHost);
	if (cudaStatus != cudaSuccess) ASSERT_TRUE(false);

	m_ParticleData = TempParticleData;

	const double Epsilon = 0.5;
	_isEqual({ -1.0, 0, 0 }, m_ParticleData[0].getPosition());
	_isNear({ -1.0, 1.0, 0 }, m_ParticleData[1].getPosition(), Epsilon);
	_isNear({ 0, 0, 0 }, m_ParticleData[2].getPosition(), Epsilon);

	double ActualArea;
	std::vector<double> ActualAngles;
	_calAreaAndAngles(ActualArea, ActualAngles);

	EXPECT_DOUBLE_EQ(ExpectArea, ActualArea);
	ASSERT_EQ(ExpectAngles.size(), ActualAngles.size());
	for (int i = 0; i < ExpectAngles.size(); i++)
	{
		EXPECT_NEAR(ExpectAngles[i], ActualAngles[i], 1e-6);
	}
}

//**************************************
//测试点：测试解算距离约束的结果是否正确
//通过条件: 
//		1、粒子的位置在一定范围
//		2、粒子间的距离变化不大
TEST_F(CTestPositionBasedDynamics, TestDistanceConstraint)
{
	m_ParticleCount = 3;
	m_ParticleData.resize(m_ParticleCount);
	_initDistanceParticles();
	double ExpectDis0 = (m_ParticleData[0].getPosition() - m_ParticleData[1].getPosition()).norm();
	double ExpectDis1 = (m_ParticleData[1].getPosition() - m_ParticleData[2].getPosition()).norm();

	m_ParticleData[0].setPosition({ -1.0, 0, 0 });

	_initCuda();

	for (int i = 0; i < 20; i++)
	{
		Animation::solveDistanceConstraints<<<1, 2>>>(m_dParticleData, m_dDistanceConstraints, m_dLocks, m_DistanceConstraintCount);
		cudaError_t cudaStatus = cudaGetLastError();
		if (cudaStatus != cudaSuccess) ASSERT_TRUE(false);
		cudaStatus = cudaDeviceSynchronize();
		if (cudaStatus != cudaSuccess) ASSERT_TRUE(false);
	}

	std::vector<Animation::CParticleData> TempParticleData(m_ParticleCount);
	// Copy output vector from GPU buffer to host memory.
	cudaError_t cudaStatus = cudaMemcpy(TempParticleData.data(), m_dParticleData, m_ParticleCount * sizeof(Animation::CParticleData), cudaMemcpyDeviceToHost);
	if (cudaStatus != cudaSuccess) ASSERT_TRUE(false);

	m_ParticleData = TempParticleData;

	double Dis0 = (m_ParticleData[0].getPosition() - m_ParticleData[1].getPosition()).norm();
	double Dis1 = (m_ParticleData[1].getPosition() - m_ParticleData[2].getPosition()).norm();
	EXPECT_NEAR(ExpectDis0, Dis0, 1e-4);
	EXPECT_NEAR(ExpectDis1, Dis1, 1e-4);
}

//**************************************
//测试点：测试解算体积约束的结果是否正确
//通过条件: 
//		1、粒子的位置在一定范围
//		2、粒子间的体积变化不大
TEST_F(CTestPositionBasedDynamics, TestVolumeConstraint)
{
	m_ParticleCount = 4;
	m_ParticleData.resize(m_ParticleCount);
	_initVolumeParticles();
	double ExpectVolume = 0.5 / 3;

	m_ParticleData[0].setPosition({ -1.0, 0, 0 });

	_initCuda();

	for (int i = 0; i < 3; i++)
	{
		Animation::solveVolumeConstraints<<<1, 1>>>(m_dParticleData, m_dVolumeConstraints, m_dLocks, m_VolumeConstraintCount);
		cudaError_t cudaStatus = cudaGetLastError();
		if (cudaStatus != cudaSuccess) ASSERT_TRUE(false);
		cudaStatus = cudaDeviceSynchronize();
		if (cudaStatus != cudaSuccess) ASSERT_TRUE(false);
	}

	std::vector<Animation::CParticleData> TempParticleData(m_ParticleCount);
	// Copy output vector from GPU buffer to host memory.
	cudaError_t cudaStatus = cudaMemcpy(TempParticleData.data(), m_dParticleData, m_ParticleCount * sizeof(Animation::CParticleData), cudaMemcpyDeviceToHost);
	if (cudaStatus != cudaSuccess) ASSERT_TRUE(false);

	m_ParticleData = TempParticleData;

	double Volume = fabs(1.0 / 6.0 * (m_ParticleData[3].getPosition() - m_ParticleData[0].getPosition()).dot((m_ParticleData[2].getPosition() - m_ParticleData[0].getPosition()).cross(m_ParticleData[1].getPosition() - m_ParticleData[0].getPosition())));
	EXPECT_NEAR(ExpectVolume, Volume, 0.05);
}