#include "pch.h"
#include <common/ConfigInterface.h>
#include "AnimationConfig.h"
#include "Voxelizer.h"
#include "GraphicsInterface.h"
#include "Sphere.h"
#include "Cuboid.h"
#include "Cylinder.h"
#include "SlopingCuboid.h"
#include "SlopingCylinder.h"

class CTestVoxelizer : public testing::Test
{
protected:
	virtual void SetUp() override
	{
		if (!m_IsConfigFileParsed)
		{
			hiveConfig::EParseResult IsParsedApplicationConfig = hiveConfig::hiveParseConfig("AnimationConfig.xml", hiveConfig::EConfigType::XML, Animation::CAnimationConfig::getInstance());
			if (IsParsedApplicationConfig != hiveConfig::EParseResult::SUCCEED)
			{
				throw std::runtime_error("Failed to parse animation config file.");
			}
			m_IsConfigFileParsed = true;
		}

		graphics::createWindow(m_pWindow, 3, 3, 1080, 720);

		m_VoxelSize = Animation::CAnimationConfig::getInstance()->getAttribute<float>(Animation::KEY_WORDS::VOXEL_SIZE);
		m_pVoxelizer = new Animation::CVoxelizer();
	}

	virtual void TearDown() override
	{
		delete m_pVoxelizer;
		graphics::closeWindow(m_pWindow);
	}

	void _initVoxelizedModel(const std::string& vModelPath, float vVoxelSize)
	{
		m_pVoxelizer->voxelize(vModelPath);
		m_AABB = m_pVoxelizer->getModelAABB();
		const glm::ivec3 Resolution = m_pVoxelizer->getVoxelResolution();
		m_ExpectedBitset.resize(Resolution.x * Resolution.y * Resolution.z, false);
		m_ActualBitset.resize(Resolution.x * Resolution.y * Resolution.z, false);
		m_ActualBitset = m_pVoxelizer->getVoxelStatusSet();
	}

	void _generateExpectedBitset(std::vector<IGeometry*> vGeometrySet)
	{
		for (const auto& pGeometry : vGeometrySet)
		{
			auto Res = pGeometry->getVoxelizationResult();
			m_ExpectedBitset |= pGeometry->getVoxelizationResult();
		}
	}

	bool _isVoxelStatusEqual() const
	{
		const boost::dynamic_bitset<> ResultBitset = (m_ActualBitset & m_ExpectedBitset) ^ m_ExpectedBitset;
		return ResultBitset.none();
	}

	static bool m_IsConfigFileParsed;
	float m_VoxelSize = 0;
	GLFWwindow *m_pWindow = nullptr;
	Animation::CVoxelizer *m_pVoxelizer = nullptr;
	Animation::SAABB m_AABB;
	boost::dynamic_bitset<> m_ActualBitset;
	boost::dynamic_bitset<> m_ExpectedBitset;
	const std::string m_TestFileDir = "../../Resource/objects/Test/";
};

bool CTestVoxelizer::m_IsConfigFileParsed = false;

//**************************************
//测试点：半径为1，中心为(0, 0, 0)的球
TEST_F(CTestVoxelizer, TestSphere)
{
	_initVoxelizedModel(m_TestFileDir + "Sphere r1.obj", m_VoxelSize);
	CSphere Sphere(m_AABB, m_VoxelSize, glm::vec3(0, 0, 0), 1.0f);
	Sphere.generateVoxelizationResult();
	_generateExpectedBitset({ &Sphere });

	EXPECT_TRUE(_isVoxelStatusEqual());
}

//**************************************
//测试点：边长为1，中心为(0, 0, 0)的立方体
TEST_F(CTestVoxelizer, TestCube)
{
	_initVoxelizedModel(m_TestFileDir + "Cube.obj", m_VoxelSize);
	CCuboid Cuboid(m_AABB, m_VoxelSize, glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, 0.5f, 0.5f));
	Cuboid.generateVoxelizationResult();
	_generateExpectedBitset({ &Cuboid });

	EXPECT_TRUE(_isVoxelStatusEqual());
}


//**************************************
//测试点：半径为1，中心为(0, 0, 0)，高为1的圆柱
TEST_F(CTestVoxelizer, TestCylinder)
{
	_initVoxelizedModel(m_TestFileDir + "Cylinder h1 r1.obj", m_VoxelSize);
	CCylinder Cylinder(m_AABB, m_VoxelSize, glm::vec3(0, 0, 0), 1, 0.5f);
	Cylinder.generateVoxelizationResult();
	_generateExpectedBitset({ &Cylinder });

	EXPECT_TRUE(_isVoxelStatusEqual());
}

//**************************************
//测试点：半径1，高1，中心(0, 0.5, 0)圆柱 上面正中心放一个边长1的正方形
TEST_F(CTestVoxelizer, R1H1C000CylinderUpA1Cube)
{
	_initVoxelizedModel(m_TestFileDir + "Cube&Clinder.obj", m_VoxelSize);
	CCylinder Cylinder(m_AABB, m_VoxelSize, glm::vec3(0, 0.5f, 0), 1, 0.5f);
	Cylinder.generateVoxelizationResult();
	CCuboid Cuboid(m_AABB, m_VoxelSize, glm::vec3(-0.5f, 1.0f, -0.5f), glm::vec3(0.5f, 2.0f, 0.5f));
	Cuboid.generateVoxelizationResult();
	_generateExpectedBitset({ &Cylinder, &Cuboid });

	EXPECT_TRUE(_isVoxelStatusEqual());
}

//**************************************
//测试点：边长1，中心(0, 0.5, 0)的正方体，上面放一个半径1的球
TEST_F(CTestVoxelizer, A1C000CubeUpR1Sphere)
{
	_initVoxelizedModel(m_TestFileDir + "Cube&Sphere.obj", m_VoxelSize);
	CCuboid Cuboid(m_AABB, m_VoxelSize, glm::vec3(-0.5f, 0.0f, -0.5f), glm::vec3(0.5f, 1.0f, 0.5f));
	Cuboid.generateVoxelizationResult();
	CSphere Sphere(m_AABB, m_VoxelSize, glm::vec3(0, 2.0f, 0), 1.0f);
	Sphere.generateVoxelizationResult();
	_generateExpectedBitset({ &Cuboid, &Sphere });

	EXPECT_TRUE(_isVoxelStatusEqual());
}

//**************************************
//测试点：边长1,中心（0，0.5，0）的正方体，上面放一个X方向为2，其余为1的长方体
TEST_F(CTestVoxelizer, A1C000CubeUpX2y1Z1Cuboid)
{
	_initVoxelizedModel(m_TestFileDir + "2Cube T.obj", m_VoxelSize);
	CCuboid Cube(m_AABB, m_VoxelSize, glm::vec3(-0.5f, 0.0f, -0.5f), glm::vec3(0.5f, 1.0f, 0.5f));
	Cube.generateVoxelizationResult();
	CCuboid Cuboid(m_AABB, m_VoxelSize, glm::vec3(-1.0f, 1.0, -0.5f), glm::vec3(1.0f, 2.0f, 0.5f));
	Cuboid.generateVoxelizationResult();
	_generateExpectedBitset({ &Cube, &Cuboid });

	EXPECT_TRUE(_isVoxelStatusEqual());
}

//**************************************
//测试点：R1 H1，中心（0，0，0）的圆柱沿X旋转90度，上面放一个R1的球
TEST_F(CTestVoxelizer, R1H1C000ZCylinderUpR1Sphere)
{
	_initVoxelizedModel(m_TestFileDir + "Sphere&XCylinder.obj", m_VoxelSize);
	CSlopingCylinder Cylinder(m_AABB, m_VoxelSize, glm::vec3(0, 0, 0), 1, 0.5f, glm::vec3(90, 0, 0));
	Cylinder.generateVoxelizationResult();
	CSphere Sphere(m_AABB, m_VoxelSize, glm::vec3(0, 2.0, 0), 1.0f);
	Sphere.generateVoxelizationResult();
	_generateExpectedBitset({ &Cylinder, &Sphere });

	EXPECT_TRUE(_isVoxelStatusEqual());
}

//**************************************
//测试点：R1 H1，中心（0，0，0）的圆柱沿X旋转90度，上面放一个边长为1的正方体
TEST_F(CTestVoxelizer, XCylinderCube)
{
	_initVoxelizedModel(m_TestFileDir + "XCylinder&Cube.obj", m_VoxelSize);
	CSlopingCylinder Cylinder(m_AABB, m_VoxelSize, glm::vec3(0, 0, 0), 1, 0.5f, glm::vec3(90, 0, 0));
	Cylinder.generateVoxelizationResult();
	CCuboid Cube(m_AABB, m_VoxelSize, glm::vec3(-0.5f, 1, -0.5f), glm::vec3(0.5f, 2.0f, 0.5f));
	Cube.generateVoxelizationResult();
	_generateExpectedBitset({ &Cylinder, &Cube });
	_generateExpectedBitset({ &Cylinder, &Cube });

	EXPECT_TRUE(_isVoxelStatusEqual());
}

//**************************************
//测试点：R1 H1，中心（0，0，0）的圆柱沿X旋转90度，延Y旋转20度，延Z旋转30度
TEST_F(CTestVoxelizer, XYZCylinderCube)
{
	_initVoxelizedModel(m_TestFileDir + "Cylinder X90Y20Z30.obj", m_VoxelSize);
	CSlopingCylinder Cylinder(m_AABB, m_VoxelSize, glm::vec3(0, 0, 0), 1, 0.5, glm::vec3(90, 20, 30));
	Cylinder.generateVoxelizationResult();
	_generateExpectedBitset({ &Cylinder });

	EXPECT_TRUE(_isVoxelStatusEqual());
}

//**************************************
//测试点：RI H1中心(0,0.5,0)的圆柱，在（0.5，1.5，0.5）和（-0.5，1.5，-0.5）放两个边长1的正方体
TEST_F(CTestVoxelizer, R1H1C000CylinderUpTwoA1Cube)
{
	_initVoxelizedModel(m_TestFileDir + "2Cube&Cylinder.obj", m_VoxelSize);
	CCylinder Cylinder(m_AABB, m_VoxelSize, glm::vec3(0, 0.5f, 0), 1, 0.5f);
	Cylinder.generateVoxelizationResult();
	CCuboid Cube1(m_AABB, m_VoxelSize, glm::vec3(0.0f, 1, 0.0f), glm::vec3(1.0f, 2.0f, 1.0f));
	Cube1.generateVoxelizationResult();
	CCuboid Cube2(m_AABB, m_VoxelSize, glm::vec3(-1.0f, 1, -1.0f), glm::vec3(0.0f, 2.0f, 0.0f));
	Cube2.generateVoxelizationResult();
	_generateExpectedBitset({ &Cylinder, &Cube1, &Cube2 });

	EXPECT_TRUE(_isVoxelStatusEqual());
}

//**************************************
//测试点：X轴45度倾斜的长2，宽1，高1的长方体
TEST_F(CTestVoxelizer, XSLOPINGCUBOID)
{
	_initVoxelizedModel(m_TestFileDir + "Cube X45.obj", m_VoxelSize);
	CSlopingCuboid Cube(m_AABB, m_VoxelSize, glm::vec3(-0.5, -1, -0.5), glm::vec3(0.5, 1, 0.5), glm::vec3(45, 0, 0));
	Cube.generateVoxelizationResult();
	_generateExpectedBitset({ &Cube });

	EXPECT_TRUE(_isVoxelStatusEqual());
}

//**************************************
//测试点：Y轴45度倾斜的长2，宽1，高1的长方体
TEST_F(CTestVoxelizer, YSLOPINGCUBOID)
{
	_initVoxelizedModel(m_TestFileDir + "Cube Y45.obj", m_VoxelSize);
	CSlopingCuboid Cube(m_AABB, m_VoxelSize, glm::vec3(-0.5, -1, -0.5), glm::vec3(0.5, 1, 0.5), glm::vec3(0, 45, 0));
	Cube.generateVoxelizationResult();
	_generateExpectedBitset({ &Cube });

	EXPECT_TRUE(_isVoxelStatusEqual());
}

//**************************************
//测试点：Z轴45度倾斜的长2，宽1，高1的长方体
TEST_F(CTestVoxelizer, ZSLOPINGCUBOID)
{
	_initVoxelizedModel(m_TestFileDir + "Cube Z45.obj", m_VoxelSize);
	CSlopingCuboid Cube(m_AABB, m_VoxelSize, glm::vec3(-0.5, -1, -0.5), glm::vec3(0.5, 1, 0.5), glm::vec3(0, 0, 45));
	Cube.generateVoxelizationResult();
	_generateExpectedBitset({ &Cube });

	EXPECT_TRUE(_isVoxelStatusEqual());
}

//**************************************
//测试点：X轴30度，Y轴60度，Z轴45度倾斜的长2，宽1，高1的长方体
TEST_F(CTestVoxelizer, XYZSLOPINGCUBOID)
{
	_initVoxelizedModel(m_TestFileDir + "Cube X30Y60Z45.obj", m_VoxelSize);
	CSlopingCuboid Cube(m_AABB, m_VoxelSize, glm::vec3(-0.5, -1, -0.5), glm::vec3(0.5, 1, 0.5), glm::vec3(30, 60, 45));
	Cube.generateVoxelizationResult();
	_generateExpectedBitset({ &Cube });

	EXPECT_TRUE(_isVoxelStatusEqual());
}

//**************************************
//测试点：X轴45度倾斜的，中心（0，0，0）长2，宽1，高1的长方体，加中心（0，2，0）的边长1的正方体,二者无碰撞
TEST_F(CTestVoxelizer, XSLOPINGCUBEANDCUBE)
{
	_initVoxelizedModel(m_TestFileDir + "XCube&Cube.obj", m_VoxelSize);
	CSlopingCuboid SlopingCuboid(m_AABB, m_VoxelSize, glm::vec3(-0.5, -1, -0.5), glm::vec3(0.5, 1, 0.5), glm::vec3(45, 0, 0));
	SlopingCuboid.generateVoxelizationResult();
	CCuboid Cube(m_AABB, m_VoxelSize, glm::vec3(-0.5, 1.5, -0.5), glm::vec3(0.5, 2.5, 0.5));
	Cube.generateVoxelizationResult();
	_generateExpectedBitset({ &Cube,&SlopingCuboid });

	EXPECT_TRUE(_isVoxelStatusEqual());
}