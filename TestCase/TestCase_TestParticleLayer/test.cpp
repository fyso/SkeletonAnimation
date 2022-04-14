#include "pch.h"
#include "ParticleLayer.h"

class TestParticleLayer : public testing::Test
{
protected:
	virtual void SetUp() override
	{
		m_pParticleLayer = new Animation::CParticleLayer();
	}

	virtual void TearDown() override
	{
		delete  m_pParticleLayer;
		m_pParticleLayer = nullptr;
	}

	Animation::CParticleLayer* m_pParticleLayer = nullptr;
};

//**************************************
//���Ե㣺�߶��������ཻ����
//ͨ������:
//	1���������߶ε�ͶӰ�������߶���
//	2�����ӵ��߶εľ���С�ڵ������Ӱ뾶
TEST_F(TestParticleLayer, TestIfIntersect) {
	Eigen::Vector3d startPos(0.0, 1.79, 0.0);
	Eigen::Vector3d endPos(-6.6, 1.79, -6.19);

	Eigen::Vector3d particlePos1(-5.35, 1.79, -5.4);
	bool result = m_pParticleLayer->isIntersect(startPos, endPos, particlePos1, 0.5);
	EXPECT_TRUE(result);

	Eigen::Vector3d particlePos2(-1.24, 1.79, -5.4);
	result = m_pParticleLayer->isIntersect(startPos, endPos, particlePos2, 0.5);
	EXPECT_FALSE(result);

	Eigen::Vector3d particlePos3(2.17, 1.79, 2.28);
	result = m_pParticleLayer->isIntersect(startPos, endPos, particlePos3, 0.5);
	EXPECT_FALSE(result);
}

//**************************************
//���Ե㣺����СK��ֵ�Ͷ�Ӧ������
TEST_F(TestParticleLayer, TestSortDistance)
{
	std::vector<int> Source{ 2,4,1,5,9,10,0 };
	std::vector<int> Result1(4);

	m_pParticleLayer->sortDistance(Source, Result1);

	EXPECT_EQ(Result1[0], 6);
	EXPECT_EQ(Result1[1], 2);
	EXPECT_EQ(Result1[2], 0);
	EXPECT_EQ(Result1[3], 1);
}