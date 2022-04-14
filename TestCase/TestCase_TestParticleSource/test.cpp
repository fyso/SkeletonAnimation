#include "pch.h"
#include "ParticleSource.h"

class CTestParticleSource : public testing::Test
{
protected:
	virtual void SetUp() override
	{
		m_pParticleSource = new Animation::CParticleSource();
	}

	void _initVoxelsBySetVoxelizationInfo()
	{
		m_VoxelizationInfo.TotalVoxelSize = { 15, 20, 30 };
		m_VoxelizationInfo.FilledVoxelSize = { 15, 19, 29 };
		m_VoxelizationInfo.StartPos = { 0.25, 0.3, 2.123 };
		m_VoxelizationInfo.Size = 0.2;

		m_pParticleSource->setVoxelizationInfo(m_VoxelizationInfo);
	}

	void _initCrossTestVoxels()
	{
		m_VoxelizationInfo.TotalVoxelSize = { 5, 5, 3 };
		m_VoxelizationInfo.FilledVoxelSize = { 5, 5, 3 };
		m_VoxelizationInfo.StartPos = { -2.0, -2.0, -1.0 };
		m_VoxelizationInfo.Size = 1.0;

		m_pParticleSource->setVoxelizationInfo(m_VoxelizationInfo);

		const std::vector<Eigen::Vector3i> Locations{ {2,2,1},{1,2,1},{0,2,1},{3,2,1},{4,2,1},
		{2,3,1},{2,4,1},{2,1,1},{2,0,1},{2,2,2},{2,2,0} };

		Eigen::Vector3d Center;
		for (auto Location : Locations)
		{
			for (int i = 0; i < 3; i++)
			{
				Center[i] = m_VoxelizationInfo.StartPos[i] + m_VoxelizationInfo.Size * Location[i];
			}
			m_pParticleSource->addVoxel(0, Location, Center);
		}

		const int VoxelCount = m_pParticleSource->getVoxelCount();
		EXPECT_EQ(Locations.size(), VoxelCount);
	}

	void _initSquareTestVoxels()
	{
		m_VoxelizationInfo.TotalVoxelSize = { 10, 10, 10 };
		m_VoxelizationInfo.FilledVoxelSize = { 10, 10, 10 };
		m_VoxelizationInfo.StartPos = { -2.05350001,10.088880,-1.0300002 };
		m_VoxelizationInfo.Size = 0.0503;

		m_pParticleSource->setVoxelizationInfo(m_VoxelizationInfo);

		std::vector<Eigen::Vector3i> Locations;
		for (int i = 0; i < 10; i++)
		{
			for (int k = 0; k < 10; k++)
			{
				for (int j = 0; j < 10; j++)
				{
					const Eigen::Vector3i Location(i, k, j);
					Locations.push_back(Location);
				}
			}
		}

		Eigen::Vector3d Center;
		for (auto Location : Locations)
		{
			for (int i = 0; i < 3; i++)
			{
				Center[i] = m_VoxelizationInfo.StartPos[i] + m_VoxelizationInfo.Size * Location[i];
			}
			m_pParticleSource->addVoxel(0, Location, Center);
		}

		const int VoxelCount = m_pParticleSource->getVoxelCount();
		EXPECT_EQ(Locations.size(), VoxelCount);
	}

	void _init4TestVoxels()
	{
		m_VoxelizationInfo.TotalVoxelSize = { 3, 1, 2 };
		m_VoxelizationInfo.FilledVoxelSize = { 3, 1, 2 };
		m_VoxelizationInfo.StartPos = { 0.0, 0.0, 0.0 };
		m_VoxelizationInfo.Size = 1.0;

		m_pParticleSource->setVoxelizationInfo(m_VoxelizationInfo);

		const std::vector<Eigen::Vector3i> Locations{ {1,0,0},{0,0,0},{1,0,1},{2,0,0} };
		std::vector<int> SplitingCount = { 1, 0, 0, 0 };

		Eigen::Vector3d Center;
		for (int i = 0; i < Locations.size(); i++)
		{
			for (int k = 0; k < 3; k++)
			{
				Center[k] = m_VoxelizationInfo.StartPos[k] + m_VoxelizationInfo.Size * Locations[i][k];
			}
			m_pParticleSource->addVoxel(SplitingCount[i], Locations[i], Center);
		}

		const int VoxelCount = m_pParticleSource->getVoxelCount();
		EXPECT_EQ(Locations.size(), VoxelCount);
	}

	std::vector<std::vector<unsigned int>> _findTetsOfVoxel(unsigned int vVoxelIndex)
	{
		std::vector<std::vector<unsigned int>> Tets;
		Animation::CTetModel TetModel = m_pParticleSource->getTetModel();
		for (int i = 0; i < TetModel.getTetSize(); i++)
		{
			if (i / 5 == vVoxelIndex)
			{
				std::vector<unsigned int> Tet = TetModel.getTet(i);
				Tets.push_back(Tet);
			}
		}

		return Tets;
	}

	bool _isNeighborTet(const std::vector<unsigned int>& vLeftTet, const std::vector<unsigned int>& vRightTet)
	{
		int NeighborVertexCount = 0;

		for (auto LeftIndex : vLeftTet)
		{
			for (auto RightIndex : vRightTet)
			{
				Eigen::Vector3d LeftParticlePos = m_pParticleSource->getParticle(LeftIndex).CenterPos;
				Eigen::Vector3d RightParticlePos = m_pParticleSource->getParticle(RightIndex).CenterPos;
				//if (fabs(LeftParticlePos[0] - RightParticlePos[0]) < DBL_EPSILON
				//	&& fabs(LeftParticlePos[1] - RightParticlePos[1]) < DBL_EPSILON
				//	&& fabs(LeftParticlePos[2] - RightParticlePos[2]) < DBL_EPSILON)
				//	NeighborVertexCount++;
				if (fabs(LeftParticlePos[0] - RightParticlePos[0]) < FLT_EPSILON
					&& fabs(LeftParticlePos[1] - RightParticlePos[1]) < FLT_EPSILON
					&& fabs(LeftParticlePos[2] - RightParticlePos[2]) < FLT_EPSILON)
					NeighborVertexCount++;
			}
		}

		return NeighborVertexCount == 3 ? true : false;
	}

	void _isEqual(const Eigen::Vector3d& vLeft, const Eigen::Vector3d& vRight)
	{
		EXPECT_DOUBLE_EQ(vLeft[0], vRight[0]);
		EXPECT_DOUBLE_EQ(vLeft[1], vRight[1]);
		EXPECT_DOUBLE_EQ(vLeft[2], vRight[2]);
	}

	void _isSameRegion(const Animation::SRegion& vLeftRegion, const Animation::SRegion& vRightRegion)
	{
		std::vector<int> LeftRegionParticles = vLeftRegion.RegionParticles;
		std::vector<int> RightRegionParticles = vRightRegion.RegionParticles;

		bool Result = m_pParticleSource->isSameParticles(LeftRegionParticles, RightRegionParticles);
		EXPECT_FALSE(Result);
	}

	virtual void TearDown() override
	{
		delete m_pParticleSource;
		m_pParticleSource = nullptr;
	}

	Animation::SVoxelizationInfo m_VoxelizationInfo;
	Animation::CParticleSource* m_pParticleSource = nullptr;
};

//**************************************
//测试点：添加体素数据
//通过条件:
//	1、根据体素位置和中心点位置正确添加体素数据
//	2、根据体素位置获得正确的中心点位置
TEST_F(CTestParticleSource, TestAddVoxel)
{
	_initVoxelsBySetVoxelizationInfo();

	const Eigen::Vector3i Location(2, 5, 7);
	const Eigen::Vector3d VoxelPos(0.5, 0.2, 1.1);
	m_pParticleSource->addVoxel(0, Location, VoxelPos);

	const Animation::SVoxelInfo VoxelInfo = m_pParticleSource->getVoxelInfo(Location);
	_isEqual(VoxelInfo.CenterPos, VoxelPos);
}

//**************************************
//测试点：根据体素中心位置获得8个顶点的位置
/*
	 *
	 *        /4|-----------/|5
	 *       /  |          / |
	 *     6|---|---------|7 |
	 *      |   |         |  |
	 *      |   |         |  |
	 *      |   |0        |  |
	 *      |  /----------|-/1
	 *      | /           |/
	 *      |-------------|
	 *      2             3
	 */
TEST_F(CTestParticleSource, TestGetParticleInitPosition)
{
	const Eigen::Vector3d VoxelCenterPos(-17.07, -0.72, 3.52);
	const double TestSize = 0.2;

	for (int i = 0; i < 8; i++)
	{
		const Eigen::Vector3d ParticlePos = m_pParticleSource->getParticleInitPos(VoxelCenterPos, TestSize, i);
		Eigen::Vector3d Expect;
		switch (i)
		{
		case 0:
			Expect = { -17.17, -0.82, 3.42 };
			break;
		case 1:
			Expect = { -16.97, -0.82, 3.42 };
			break;
		case 2:
			Expect = { -17.17, -0.82, 3.62 };
			break;
		case 3:
			Expect = { -16.97, -0.82, 3.62 };
			break;
		case 4:
			Expect = { -17.17, -0.62, 3.42 };
			break;
		case 5:
			Expect = { -16.97, -0.62, 3.42 };
			break;
		case 6:
			Expect = { -17.17, -0.62, 3.62 };
			break;
		case 7:
			Expect = { -16.97, -0.62, 3.62 };
			break;
		}

		_isEqual(ParticlePos, Expect);
	}
}

//**************************************
//测试点：根据所有体素生成所有粒子
/*              6
	 *          |
	 *          5  9
	 *          |/
	 *    2--1--0--3--4
	 *         /|
	 *      10  7
	 *          |
	 *          8
	 */
	 //通过条件:
	 //	1、生成的粒子的数量正确
	 //	2、没有重复的粒子
	 //	3、体素与粒子的索引关系正确
TEST_F(CTestParticleSource, TestCreateParticles)
{
	_initCrossTestVoxels();
	m_pParticleSource->createParticlesFromVoxels();

	const int ParticleCount = m_pParticleSource->getParticleCount();
	EXPECT_EQ(48, ParticleCount);

	const int VoxelCount = m_pParticleSource->getVoxelCount();
	for (int i = 0; i < VoxelCount; i++)
	{
		const Eigen::Vector3i VoxelLocation = m_pParticleSource->getVoxelLocation(i);

		const Animation::SVoxelInfo VoxelInfo = m_pParticleSource->getVoxelInfo(VoxelLocation);
		EXPECT_TRUE(VoxelInfo.IsTracked);

		for (int k = 0; k < 8; k++)
		{
			const int ParticleIndex = VoxelInfo.RelatedParticles[k];
			const Animation::SParticle Particle = m_pParticleSource->getParticle(ParticleIndex);
			const int RelatedVoxelCount = Particle.RelatedVoxels.size();
			EXPECT_TRUE(RelatedVoxelCount > 0 && RelatedVoxelCount < 5);

			bool IsFoundVoxel = false;
			for (int m = 0; m < RelatedVoxelCount; m++)
			{
				const Eigen::Vector3i RelatedVoxelLocation = Particle.RelatedVoxels[m];
				if (RelatedVoxelLocation == VoxelLocation)
				{
					IsFoundVoxel = true;
					break;
				}
			}

			EXPECT_TRUE(IsFoundVoxel);
		}
	}
}

//**************************************
//测试点：根据所有体素生成所有粒子
TEST_F(CTestParticleSource, TestCreateSquareParticles)
{
	_initSquareTestVoxels();
	m_pParticleSource->createParticlesFromVoxels();

	const int ParticleCount = m_pParticleSource->getParticleCount();
	EXPECT_EQ(1331, ParticleCount);

	int Count = 0;
	for (int i = 0; i < ParticleCount; i++)
	{
		Animation::SParticle Particle = m_pParticleSource->getParticle(i);
		if (Particle.RelatedVoxels.size() == 8)
			Count++;
	}
	EXPECT_EQ(729, Count);
}

//**************************************
//测试点：生成所有粒子的一环邻居
//通过条件:
//	1、皮肤粒子的数量正确
//	2、一环邻居的数量正确
//	3、一环邻居关系是双向的，即如果A是B的其中一个邻居，那么A的一环邻居里也有B
//  4、粒子到邻居的直线距离大于等于一个单位长度，小于两个单位长度
TEST_F(CTestParticleSource, TestGenerateOneRingNeighbors)
{
	_initCrossTestVoxels();
	m_pParticleSource->createParticlesFromVoxels();
	m_pParticleSource->generateOneRingNeighbors();

	const int SkinParticleCount = m_pParticleSource->getSkinParticleCount();
	EXPECT_EQ(48, SkinParticleCount);

	int ParticleCountOf7Neighbors = 0, ParticleCountOf11Neighbors = 0, ParticleCountOf19Neighbors = 0;
	const int ParticleCount = m_pParticleSource->getParticleCount();
	for (int i = 0; i < ParticleCount; i++)
	{
		std::vector<int> Neighbors;
		m_pParticleSource->dumpOneRingNeighbors(i, Neighbors);
		EXPECT_TRUE(Neighbors.size() > 0);

		const int NeighborCount = Neighbors.size();
		for (int k = 0; k < NeighborCount; k++)
		{
			const Eigen::Vector3d Offset = m_pParticleSource->getParticlePos(i) - m_pParticleSource->getParticlePos(Neighbors[k]);
			EXPECT_TRUE(Offset.norm() >= m_VoxelizationInfo.Size - DBL_EPSILON);
			EXPECT_TRUE(Offset.norm() < 2 * m_VoxelizationInfo.Size);

			std::vector<int> CurNeighbors;
			m_pParticleSource->dumpOneRingNeighbors(Neighbors[k], CurNeighbors);
			auto Itr = std::find(CurNeighbors.begin(), CurNeighbors.end(), i);
			EXPECT_TRUE(Itr != CurNeighbors.end());
		}

		switch (NeighborCount)
		{
		case 7:
			ParticleCountOf7Neighbors++;
			break;
		case 11:
			ParticleCountOf11Neighbors++;
			break;
		case 19:
			ParticleCountOf19Neighbors++;
			break;
		default:
			EXPECT_TRUE(false);
			break;
		}
	}

	EXPECT_EQ(24, ParticleCountOf7Neighbors);
	EXPECT_EQ(16, ParticleCountOf11Neighbors);
	EXPECT_EQ(8, ParticleCountOf19Neighbors);
}

//**************************************
//测试点：生成所有粒子的x+-,y+-,z+-方向的邻居
//通过条件:
//	1、邻居数量不超过6
//  2、粒子到邻居的直线距离为一个单位长度
//	3、邻居关系是双向的
TEST_F(CTestParticleSource, TestGenerateCommonNeighbors)
{
	_initCrossTestVoxels();
	m_pParticleSource->createParticlesFromVoxels();
	m_pParticleSource->generateOneRingNeighbors();
	m_pParticleSource->generateCommonNeighbors();

	const int ParticleCount = m_pParticleSource->getParticleCount();
	for (int i = 0; i < ParticleCount; i++)
	{
		std::vector<int> Neighbors;
		m_pParticleSource->dumpCommonNeighbors(i, Neighbors);
		EXPECT_TRUE(Neighbors.size() > 0);

		const int NeighborCount = Neighbors.size();
		for (int k = 0; k < NeighborCount; k++)
		{
			Eigen::Vector3d Offset = m_pParticleSource->getParticlePos(i) - m_pParticleSource->getParticlePos(Neighbors[k]);
			EXPECT_TRUE(fabs(Offset.norm() - m_VoxelizationInfo.Size) < DBL_EPSILON);

			std::vector<int> CurNeighbors;
			m_pParticleSource->dumpCommonNeighbors(Neighbors[k], CurNeighbors);
			auto Itr = std::find(CurNeighbors.begin(), CurNeighbors.end(), i);
			EXPECT_TRUE(Itr != CurNeighbors.end());
		}
	}
}

//**************************************
//测试点：根据体素生成的四面体
/*
*      2
*      |
*   0--1--3
*/
//通过条件: 
//1、四面体数量应该是体素数量的5倍
//2、相邻的体素之间至少有一对四面体共享三个相同顶点
TEST_F(CTestParticleSource, TestGenerateTets)
{
	_init4TestVoxels();
	m_pParticleSource->createParticlesFromVoxels();

	const int VoxelCount = m_pParticleSource->getVoxelCount();
	const int TetSize = m_pParticleSource->getTetSize();
	EXPECT_TRUE(TetSize / 5 == VoxelCount);

	std::vector<std::vector<unsigned int>> VoxelNeighbors(4);
	VoxelNeighbors[0] = { 1 };
	VoxelNeighbors[1] = { 0, 2, 3 };
	VoxelNeighbors[2] = { 1 };
	VoxelNeighbors[3] = { 1 };

	for (int i = 0; i < VoxelNeighbors.size(); i++)
	{
		const unsigned int LeftVoxel = i;
		for (int k = 0; k < VoxelNeighbors[i].size(); k++)
		{
			const unsigned int RightVoxel = VoxelNeighbors[i][k];

			std::vector<std::vector<unsigned int>> LeftTets = _findTetsOfVoxel(LeftVoxel);
			std::vector<std::vector<unsigned int>> RightTets = _findTetsOfVoxel(RightVoxel);

			int NeighborCount = 0;
			for (auto LeftTet : LeftTets)
			{
				for (auto RightTet : RightTets)
				{
					if (_isNeighborTet(LeftTet, RightTet))
						NeighborCount++;
				}
			}
			EXPECT_TRUE(NeighborCount > 0);
		}
	}
}

//**************************************
//测试点：生成所有形状匹配约束的区域
//通过条件:
//	1、区域数量正确
//  2、区域内粒子数量正确
//	3、任意两个区域不相同
TEST_F(CTestParticleSource, TestGenerateRegions)
{
	_init4TestVoxels();
	m_pParticleSource->initialize();

	const int RegionCount = m_pParticleSource->getRegionCount();
	EXPECT_EQ(7, RegionCount);

	int RegionCountOf8Particles = 0, RegionCountOf12Partciles = 0, RegionCountOf16Partciles = 0;
	std::vector<Animation::SRegion> Regions(RegionCount);
	for (int i = 0; i < RegionCount; i++)
	{
		Regions[i] = m_pParticleSource->getRegion(i);
		switch (Regions[i].RegionParticles.size())
		{
		case 8:
			RegionCountOf8Particles++;
			break;
		case 12:
			RegionCountOf12Partciles++;
			break;
		case 16:
			RegionCountOf16Partciles++;
			break;
		default:
			EXPECT_TRUE(false);
			break;
		}
	}
	EXPECT_EQ(3, RegionCountOf8Particles);
	EXPECT_EQ(2, RegionCountOf12Partciles);
	EXPECT_EQ(2, RegionCountOf16Partciles);

	for (int i = 0; i < RegionCount - 1; i++)
	{
		for (int k = i + 1; k < RegionCount; k++)
		{
			_isSameRegion(Regions[i], Regions[k]);
		}
	}
}

//**************************************
//测试点：测试自定义key值的unordered_map
TEST(TestHash, testHashUnorderedMap)
{
	std::unordered_map<Eigen::Vector3i, Animation::SVoxelInfo, Animation::HashFuncVector3i, Animation::EqualKeyVector3i> HashVoxels;
	Eigen::Vector3i VoxelLocation(2, 5, 1);
	Eigen::Vector3d Pos(-17.07184, -0.7276255, 3.527293);
	Animation::SVoxelInfo TestVoxel(Pos);
	HashVoxels.insert(std::make_pair(VoxelLocation, TestVoxel));
	auto Itr = HashVoxels.find(VoxelLocation);
	bool Result = (Itr != HashVoxels.end());
	EXPECT_TRUE(Result);
	EXPECT_EQ(Itr->second.CenterPos, Pos);
}