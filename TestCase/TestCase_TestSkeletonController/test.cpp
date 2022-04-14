#include "pch.h"
#include "SkeletonController.h"

typedef int ParentIndex;
typedef Eigen::Vector3d WorldPos;

class TestSkeletonController : public testing::Test
{
protected:
	virtual void SetUp() override
	{
		std::string FileName = "TestBoneTransformData.csv";
		m_pSkeletonController = new Animation::CSkeletonController(m_DirPath + FileName);
	}

	virtual void TearDown() override
	{
		delete m_pSkeletonController;
		m_pSkeletonController = nullptr;
	}

	void _initExpectSkeletonInfo(std::vector<Animation::SSkeletonDefinition>& voSkeletonInfo)
	{
		voSkeletonInfo.resize(4);

		Animation::SSkeletonDefinition SkeletonDefinition0;
		SkeletonDefinition0.ParentIndex = -1;
		SkeletonDefinition0.BoneID = 0;
		SkeletonDefinition0.Offset = Eigen::Vector3d(-0.002738003, 0.8904486, 0.006547624);
		voSkeletonInfo[0] = SkeletonDefinition0;

		Animation::SSkeletonDefinition SkeletonDefinition1;
		SkeletonDefinition1.ParentIndex = 0;
		SkeletonDefinition1.BoneID = 1;
		SkeletonDefinition1.Offset = Eigen::Vector3d(-0.09700106, -0.06611533, -0.000552175);
		voSkeletonInfo[1] = SkeletonDefinition1;

		Animation::SSkeletonDefinition SkeletonDefinition2;
		SkeletonDefinition2.ParentIndex = 0;
		SkeletonDefinition2.BoneID = 2;
		SkeletonDefinition2.Offset = Eigen::Vector3d(0.1027032, -0.06611688, -0.000551922);
		voSkeletonInfo[2] = SkeletonDefinition2;

		Animation::SSkeletonDefinition SkeletonDefinition3;
		SkeletonDefinition3.ParentIndex = 1;
		SkeletonDefinition3.BoneID = 3;
		SkeletonDefinition3.Offset = Eigen::Vector3d(1.86E-08, 0.3406222, 7.2E-09);
		voSkeletonInfo[3] = SkeletonDefinition3;
	}

	void _initLocalToWorldTransfer(std::vector<std::vector<Eigen::Matrix4d>>& voLocalToWorld)
	{
		voLocalToWorld.resize(3);

		std::vector<Eigen::Matrix4d> Transfer0(4);
		Transfer0[0] << 19.99999, -0.00576664, 0.01377385, -0.03561825, 0.00578148, 19.99998, -0.02155099, 17.82717, -0.01376763, 0.02155496, 19.99998, 0.4362797, 0, 0, 0, 1;
		Transfer0[1] << -19.86094, -2.120829, -1.022979, -1.975265, 2.222834, -19.75366, -2.202709, 16.50432, -0.7768002, -2.301081, 19.85202, 0.4251465, 0, 0, 0, 1;
		Transfer0[2] << -19.90479, 1.811632, 0.7218614, 2.01882, -1.855357, -19.87211, -1.287263, 16.50544, 0.6006433, -1.34809, 19.94555, 0.4224021, 0, 0, 0, 1;
		Transfer0[3] << -19.98603, -0.6990924, 0.2665921, -2.697668, 0.640243, -19.66725, -3.576281, 9.775779, 0.3871642, -3.56524, 19.67589, -0.3586529, 0, 0, 0, 1;
		voLocalToWorld[0] = Transfer0;

		std::vector<Eigen::Matrix4d> Transfer1(4);
		Transfer1[0] << 19.99724, 0.1281923, 0.3063676, -0.1753734, -0.1158019, 19.98353, -0.8030143, 17.75109, -0.3112625, 0.8011296, 19.98152, 1.165203, 0, 0, 0, 1;
		Transfer1[1] << -19.88883, -1.837073, -1.029973, -2.123772, 1.96743, -19.69639, -2.860366, 16.44155, -0.7516015, -2.945779, 19.76761, 1.131396, 0, 0, 0, 1;
		Transfer1[2] << -19.88954, 1.961361, 0.7502028, 1.869764, -2.027854, -19.79525, -2.008938, 16.4184, 0.5455104, -2.07389, 19.88478, 1.069239, 0, 0, 0, 1;
		Transfer1[3] << -19.98866, -0.6673932, 0.09618718, -2.749521, 0.6225277, -19.36164, -4.973884, 9.732525, 0.2590943, -4.968059, 19.37144, 0.1279985, 0, 0, 0, 1;
		voLocalToWorld[1] = Transfer1;

		std::vector<Eigen::Matrix4d> Transfer2(4);
		Transfer2[0] << 19.99685, 0.1396672, 0.3261382, -0.2002565, -0.1250198, 19.97976, -0.8907685, 17.73966, -0.3320287, 0.8885894, 19.97749, 1.244835, 0, 0, 0, 1;
		Transfer2[1] << -19.89143, -1.798414, -1.047839, -2.149387, 1.93437, -19.68992, -2.926658, 16.43131, -0.7684265, -3.012107, 19.75697, 1.207261, 0, 0, 0, 1;
		Transfer2[2] << -19.88659, 1.990225, 0.7524692, 1.844071, -2.059114, -19.78326, -2.093475, 16.40631, 0.5359909, -2.159057, 19.87597, 1.140958, 0, 0, 0, 1;
		Transfer2[3] << -19.98961, -0.640665, 0.07754938, -2.761967, 0.5998741, -19.33265, -5.088142, 9.724482, 0.2379516, -5.083163, 19.34183, 0.1812709, 0, 0, 0, 1;
		voLocalToWorld[2] = Transfer2;
	}

	void _generateAllExpectPose(const std::vector<Animation::SSkeletonDefinition>& vSkeleton, const std::vector<std::vector<Eigen::Matrix4d>>& vAllLocalToWorldTransfer, std::vector<std::vector<std::pair<ParentIndex, WorldPos>>>& voAllPose)
	{
		auto LocalToWorldTransfersSize = vAllLocalToWorldTransfer.size();

		voAllPose.resize(LocalToWorldTransfersSize);
		for (int i = 0; i < LocalToWorldTransfersSize; i++)
		{
			std::vector<std::pair<ParentIndex, WorldPos>> TempPose(vAllLocalToWorldTransfer[i].size());
			for (int k = 0; k < vAllLocalToWorldTransfer[i].size(); k++)
			{
				std::pair<ParentIndex, WorldPos> TempInfo;
				TempInfo.first = vSkeleton[k].ParentIndex;
				float X = vAllLocalToWorldTransfer[i][k](0, 3);
				float Y = vAllLocalToWorldTransfer[i][k](1, 3);
				float Z = vAllLocalToWorldTransfer[i][k](2, 3);
				TempInfo.second = WorldPos(X, Y, Z);

				TempPose[k] = TempInfo;
			}
			voAllPose[i] = TempPose;
		}
	}

	const std::string m_DirPath = "../../Resource/";
	Animation::CSkeletonController* m_pSkeletonController = nullptr;
};

//**************************************
//测试点: 从文件加载骨骼数据操作
//通过条件: 
//	1、错误的文件路径或者文件内容不符合格式要求时加载失败，返回false
//	2、文件路径和内容都正确时加载成功，返回true
TEST_F(TestSkeletonController, TestInitSkeleton)
{
	std::string FileName = "";
	EXPECT_FALSE(m_pSkeletonController->initSkeletonByFile(m_DirPath + FileName));

	FileName = "InvalidFileName.csv";
	EXPECT_FALSE(m_pSkeletonController->initSkeletonByFile(FileName));

	FileName = "TestEmptyData.csv";
	EXPECT_FALSE(m_pSkeletonController->initSkeletonByFile(m_DirPath + FileName));

	FileName = "TestInvalidBoneTransformData.csv";
	EXPECT_FALSE(m_pSkeletonController->initSkeletonByFile(m_DirPath + FileName));

	FileName = "Manny20new_boneTransformData.csv";
	EXPECT_TRUE(m_pSkeletonController->initSkeletonByFile(m_DirPath + FileName));
}

//**************************************
//测试点: 生成骨骼动画所需数据
//通过条件: 
//	1、正确生成骨骼数据
//	2、正确生成从运动开始到运动结束的所有姿势信息
//	3、正确生成初始骨骼的世界空间坐标
//	4、正确生成从世界空间到模型空间的转换矩阵
TEST_F(TestSkeletonController, TestGenerateData)
{
	std::vector<Animation::SSkeletonDefinition> Skeleton;
	std::vector<std::vector<Eigen::Matrix4d>> AllLocalToWorld;
	_initExpectSkeletonInfo(Skeleton);
	_initLocalToWorldTransfer(AllLocalToWorld);

	std::vector<Animation::SSkeletonDefinition> TempSkeleton = m_pSkeletonController->getSkeleton();
	ASSERT_EQ(Skeleton.size(), TempSkeleton.size());
	for (int i = 0; i < Skeleton.size(); i++)
	{
		EXPECT_EQ(Skeleton[i], TempSkeleton[i]);
	}

	std::vector<std::vector<Eigen::Matrix4d>> TempAllPose = m_pSkeletonController->getAllPose();
	ASSERT_EQ(AllLocalToWorld.size(), TempAllPose.size());
	for (int i = 0; i < AllLocalToWorld.size(); i++)
	{
		std::vector<Eigen::Matrix4d> ExpectPose = AllLocalToWorld[i];

		ASSERT_EQ(ExpectPose.size(), TempAllPose[i].size());
		for (int k = 0; k < ExpectPose.size(); k++)
		{
			EXPECT_TRUE(ExpectPose[k] == TempAllPose[i][k]);
		}
	}

	std::vector<Eigen::Vector3d> ExpectInitialWorldPos;
	for (int i = 0; i < AllLocalToWorld[0].size(); i++)
	{
		Eigen::Matrix4d Transfer = AllLocalToWorld[0][i];
		ExpectInitialWorldPos.push_back(Eigen::Vector3d(Transfer(0, 3), Transfer(1, 3), Transfer(2, 3)));
	}

	std::vector<Eigen::Vector3d> TempInitialWorldPos = m_pSkeletonController->getInitialWorldPosition();
	ASSERT_EQ(ExpectInitialWorldPos.size(), TempInitialWorldPos.size());
	for (int i = 0; i < ExpectInitialWorldPos.size(); ++i)
	{
		EXPECT_EQ(ExpectInitialWorldPos[i], TempInitialWorldPos[i]);
	}

	std::vector<Eigen::Matrix4d> ExpectLocalToWorldTransfer = AllLocalToWorld[0];
	std::vector<Eigen::Matrix4d> TempWorldToLocalTransfer = m_pSkeletonController->getWorldToLocalTransfer();
	ASSERT_EQ(ExpectLocalToWorldTransfer.size(), TempWorldToLocalTransfer.size());
	for (int k = 0; k < ExpectLocalToWorldTransfer.size(); k++)
	{
		EXPECT_TRUE(ExpectLocalToWorldTransfer[k].inverse() == TempWorldToLocalTransfer[k]);
	}
}

//**************************************
//测试点: 获取骨骼运动过程中下一个姿势的数据
TEST_F(TestSkeletonController, TestNextPose)
{
	std::vector<std::vector<Eigen::Matrix4d>> AllLocalToWorld;
	_initLocalToWorldTransfer(AllLocalToWorld);

	m_pSkeletonController->resetPose();
	for (int i = 0; i < AllLocalToWorld.size(); i++)
	{
		std::vector<Eigen::Matrix4d> CurrentPose;
		m_pSkeletonController->dumpNextPose(CurrentPose);
		ASSERT_EQ(AllLocalToWorld[i].size(), CurrentPose.size());
		for (int k = 0; k < AllLocalToWorld[i].size(); k++)
		{
			EXPECT_TRUE(AllLocalToWorld[i][k] == CurrentPose[k]);
		}
	}
}

//**************************************
//测试点: 测试类的get函数返回值
//通过条件: 
//	1、正确返回骨骼节点数
//	2、正确返回每个骨骼的父节点索引值
//	3、正确返回每个骨骼节点的初始世界空间坐标
//	4、正确返回初始状态从世界空间到模型空间的转换矩阵
TEST_F(TestSkeletonController, TestGetMethod)
{
	EXPECT_EQ(4, m_pSkeletonController->getBoneCount());

	std::vector<Animation::SSkeletonDefinition> Skeleton;
	_initExpectSkeletonInfo(Skeleton);
	for (int i = 0; i < Skeleton.size(); i++)
	{
		EXPECT_EQ(Skeleton[i].ParentIndex, m_pSkeletonController->getParentIndexAt(i));
	}

	std::vector<std::vector<Eigen::Matrix4d>> AllLocalToWorld;
	_initLocalToWorldTransfer(AllLocalToWorld);
	ASSERT_EQ(Skeleton.size(), AllLocalToWorld[0].size());
	for (int i = 0; i < Skeleton.size(); i++)
	{
		Eigen::Matrix4d Transfer = AllLocalToWorld[0][i];
		Eigen::Vector3d CurrentPos = Eigen::Vector3d(Transfer(0, 3), Transfer(1, 3), Transfer(2, 3));

		EXPECT_EQ(CurrentPos, m_pSkeletonController->getInitialBoneWorldPos(i));
	}

	for (int i = 0; i < AllLocalToWorld[0].size(); i++)
	{
		EXPECT_TRUE(AllLocalToWorld[0][i].inverse() == m_pSkeletonController->getWorldToLocalTransfer(i));
	}
}