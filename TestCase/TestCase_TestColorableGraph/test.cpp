#include "pch.h"
#include "ColorableGraph.h"
#include <boost/filesystem.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/access.hpp>
#include <fstream>

using namespace Animation;

/*
		  6
		  |  \
	  1---3---5
	  | \ |\ /|
	  0---2---4
*/
class CColorableGraphUnitTest : public testing::Test
{
protected:
	virtual void SetUp()
	{
		m_Graph = CColorableGraphPtr(new CColorableGraph(7, 4));
		m_Graph->addEdgeV(0, 1);
		m_Graph->addEdgeV(0, 2);
		m_Graph->addEdgeV(1, 2);
		m_Graph->addEdgeV(1, 3);
		m_Graph->addEdgeV(2, 3);
		m_Graph->addEdgeV(2, 4);
		m_Graph->addEdgeV(2, 5);
		m_Graph->addEdgeV(3, 4);
		m_Graph->addEdgeV(3, 5);
		m_Graph->addEdgeV(3, 6);
		m_Graph->addEdgeV(4, 5);
		m_Graph->addEdgeV(5, 6);

		srand((unsigned)time(NULL));

	}
	CColorableGraphPtr m_Graph;

	int TestCount = 10;
	int MinNodeCount = 10;
	int MaxNodeCount = 200;
	float SparseRate = 0.2f;
	int MaxClique = 27;
	int SparseRatePrecision = 999;

	CColorableGraphPtr _generateRandomGraph() {
		int NodeCount = (rand() % (MaxNodeCount - MinNodeCount + 1)) + MinNodeCount;
		CColorableGraphPtr Graph = CColorableGraphPtr(new CColorableGraph(NodeCount));
		for (int NodeStart = 0; NodeStart < NodeCount; NodeStart++)
		{
			if (Graph->getNeighborOfNodeV(NodeStart).size() > MaxClique) continue;

			for (int NodeEnd = 0; NodeEnd < NodeCount; NodeEnd++)
			{
				if (NodeStart == NodeEnd) continue;

				float NeedAnEdge = rand() % (SparseRatePrecision + 1) / (float)(SparseRatePrecision + 1);
				if (NeedAnEdge > SparseRate)
				{
					Graph->addEdgeV(NodeStart, NodeEnd);
				}
			}
		}
		return Graph;
	}
};

TEST_F(CColorableGraphUnitTest, FunctionInitializeGraph)
{
	m_Graph->setEdgeWeight(3, 6, 10);
	unsigned int Weight = m_Graph->getEdgeWeight(3, 6);
	ASSERT_EQ(Weight, 10);

	m_Graph->addEdgeV(0, 1);
	Weight = m_Graph->getEdgeWeight(0, 1);
	ASSERT_EQ(Weight, 2);

	ASSERT_EQ(m_Graph->getTotalEdgeWeight(), 22);

	m_Graph->deleteEdgeV(1, 6);
	ASSERT_FALSE(m_Graph->hasEdgeV(1, 6));
	ASSERT_TRUE(m_Graph->hasEdgeV(0, 1));
	ASSERT_EQ(m_Graph->getNeighborOfNodeV(1).size(), 3);

	std::set<int> NeighborsOfNode1 = m_Graph->getNeighborOfNodeV(1);
	for (int Neighbor : NeighborsOfNode1)
	{
		ASSERT_TRUE(Neighbor == 0 || Neighbor == 2 || Neighbor == 3);
	}

	m_Graph->deleteEdgeV(1, 2);
	m_Graph->deleteEdgeV(1, 3);
	NeighborsOfNode1 = m_Graph->getNeighborOfNodeV(1);
	for (int Neighbor : NeighborsOfNode1)
	{
		ASSERT_TRUE(Neighbor == 0);
	}

	m_Graph->deleteEdgeV(1, 0);
	NeighborsOfNode1 = m_Graph->getNeighborOfNodeV(1);
	ASSERT_TRUE(NeighborsOfNode1.size() == 0);
}

//TEST_F(CColorableGraphUnitTest, FunctionSearchMaximalCliques_RandomTesting)
//{
//	std::vector<CColorableGraph> ErrorTestCaseses;
//	if (!boost::filesystem::exists("./Error_Graph_TestCase_Data"))
//	{
//		for (int i = 0; i < TestCount; i++)
//		{
//			CColorableGraphPtr Graph = _generateRandomGraph();
//			int Errored = false;
//			Graph->searchMaximalCliquesByBronKerbosch([&](const std::set<int>& vInputClique)->void
//			{
//				for (auto NodeStart : vInputClique)
//				{
//					for (auto NodeEnd : vInputClique)
//					{
//						if (!Graph->hasEdgeV(NodeStart, NodeEnd))
//						{
//							Errored = true;
//							return;
//						}
//					}
//				}
//			});
//			if (Errored)
//			{
//				ErrorTestCaseses.push_back(*Graph);
//			}
//		}
//		if (ErrorTestCaseses.size() == 0)
//		{
//			ASSERT_FALSE(false);
//		}
//		else
//		{
//			ASSERT_FALSE(true);
//			std::fstream CacheFile("./Error_Graph_TestCase_Data", std::fstream::out);
//			if (CacheFile)
//			{
//				boost::archive::binary_oarchive oa(CacheFile);
//				oa << ErrorTestCaseses;
//			}
//			CacheFile.close();
//		}
//	}
//	else
//	{
//		std::vector<CColorableGraph> TempErrorTestCaseses;
//		std::fstream CacheFile("./Error_Graph_TestCase_Data", std::fstream::in);
//		if (CacheFile)
//		{
//			boost::archive::binary_iarchive ia(CacheFile);
//			ia >> TempErrorTestCaseses;
//		}
//		CacheFile.close();
//
//		for (auto Graph : TempErrorTestCaseses)
//		{
//			int Errored = false;
//			Graph.searchMaximalCliquesByBronKerbosch([&](const std::set<int>& vInputClique)->void
//			{
//				for (auto NodeStart : vInputClique)
//				{
//					for (auto NodeEnd : vInputClique)
//					{
//						if (!Graph.hasEdgeV(NodeStart, NodeEnd))
//						{
//							Errored = true;
//							return;
//						}
//					}
//				}
//			});
//			if (Errored)
//			{
//				ErrorTestCaseses.push_back(Graph);
//			}
//		}
//	}
//
//	ASSERT_EQ(ErrorTestCaseses.size(), 0);
//}

/*
		  6

	  1   3---5
	  |   |\ /|
	  0   2---4
*/
TEST_F(CColorableGraphUnitTest, FunctionSearchMaximalCliques_NoConnectedGraph)
{
	m_Graph->deleteEdgeV(1, 3);
	m_Graph->deleteEdgeV(1, 2);
	m_Graph->deleteEdgeV(0, 2);
	m_Graph->deleteEdgeV(3, 6);
	m_Graph->deleteEdgeV(5, 6);
	m_Graph->setColorNumberQ(0);

	m_Graph->searchMaximalCliquesByBronKerbosch([&](const std::vector<int>& vInputClique)->void
	{
		std::vector<int> Result;
		if (find(vInputClique.begin(),vInputClique.end(),6)!= vInputClique.end())
		{
			Result.push_back(6);
		}
		if (find(vInputClique.begin(), vInputClique.end(), 1) != vInputClique.end())
		{
			Result.push_back(0);
			Result.push_back(1);
		}
		if (find(vInputClique.begin(), vInputClique.end(), 2) != vInputClique.end())
		{
			Result.push_back(2);
			Result.push_back(3);
			Result.push_back(4);
			Result.push_back(5);
		}
		sort(Result.begin(), Result.end());
		std::vector<int> sortClique = vInputClique;
		sort(sortClique.begin(), sortClique.end());

		ASSERT_TRUE(sortClique == Result);
	});
}

/*
              9
			  |
		      6
		      |  \
  10--7---1---3---5
	\ |\ /| \ |\ /|
	  8---0---2---4
*/
TEST_F(CColorableGraphUnitTest, FunctionSearchMaximalCliques)
{
	CColorableGraphPtr Graph = CColorableGraphPtr(new CColorableGraph(11));
	Graph->addEdgeV(0, 1);
	Graph->addEdgeV(0, 2);
	Graph->addEdgeV(1, 2);
	Graph->addEdgeV(1, 3);
	Graph->addEdgeV(2, 3);
	Graph->addEdgeV(2, 4);
	Graph->addEdgeV(2, 5);
	Graph->addEdgeV(3, 4);
	Graph->addEdgeV(3, 5);
	Graph->addEdgeV(3, 6);
	Graph->addEdgeV(4, 5);
	Graph->addEdgeV(5, 6);
	Graph->addEdgeV(7, 1);
	Graph->addEdgeV(0, 8);
	Graph->addEdgeV(8, 7);
	Graph->addEdgeV(7, 0);
	Graph->addEdgeV(8, 1);
	Graph->addEdgeV(6, 9);
	Graph->addEdgeV(10,7);
	Graph->addEdgeV(10,8);

	Graph->searchMaximalCliquesByBronKerbosch([&](const std::vector<int>& vInputClique)->void{}); 
	std::vector<std::vector<int>> Cliques = Graph->getClique();
	ASSERT_EQ(Cliques.size(), 3);
	ASSERT_EQ(Cliques[0].size(), 4);
	ASSERT_EQ(Cliques[1].size(), 4);
	ASSERT_EQ(Cliques[2].size(), 2);
}

/*
			  8
			  |
			  5
			  |  
              2---4
	            / |
		  7---1---3
        / |\ /|
	  9---6---0
*/
TEST_F(CColorableGraphUnitTest, FunctionSearchMaximalCliques2)
{
	CColorableGraphPtr Graph = CColorableGraphPtr(new CColorableGraph(10));
	Graph->addEdgeV(1, 3);
	Graph->addEdgeV(1, 4);
	Graph->addEdgeV(1, 0);
	Graph->addEdgeV(1, 7);
	Graph->addEdgeV(1, 6);
	Graph->addEdgeV(2, 4);
	Graph->addEdgeV(2, 5);
	Graph->addEdgeV(3, 4);
	Graph->addEdgeV(6, 0);
	Graph->addEdgeV(0, 7);
	Graph->addEdgeV(7, 6);
	Graph->addEdgeV(6, 0);
	Graph->addEdgeV(7, 0);
	Graph->addEdgeV(5, 8);
	Graph->addEdgeV(9, 6);
	Graph->addEdgeV(9, 7);

	Graph->searchMaximalCliquesByBronKerbosch([&](const std::vector<int>& vInputClique)->void {});
	std::vector<std::vector<int>> Cliques = Graph->getClique();
	ASSERT_EQ(Cliques[0].size(), 4);
	ASSERT_EQ(Cliques[1].size(), 2);
	ASSERT_EQ(Cliques[2].size(), 2);
}

/*
		  6

	  1   3---5
	  |   |\/ | \
	  0   2---4---7
*/
TEST_F(CColorableGraphUnitTest, FunctionSearchMaximalCliquesInChildGraph)
{
	CColorableGraph testGraph(8);
	testGraph.addEdgeV(0, 1);
	testGraph.addEdgeV(2, 3);
	testGraph.addEdgeV(2, 4);
	testGraph.addEdgeV(2, 5);
	testGraph.addEdgeV(3, 4);
	testGraph.addEdgeV(3, 5);
	testGraph.addEdgeV(4, 5);
	testGraph.addEdgeV(4, 7);
	testGraph.addEdgeV(5, 7);

	testGraph.setColorNumberQ(3);
	std::vector<int> childGraph = { 4,5,7 };
	bool result = testGraph.searchCliqueInChildGraph(childGraph);
	ASSERT_FALSE(result);

	testGraph.setColorNumberQ(3);
	std::vector<int> childGraph2 = { 2,4,5 };
	bool result2 = testGraph.searchCliqueInChildGraph(childGraph2);
	ASSERT_FALSE(result2);

	int edgeCount = testGraph.getEdgeCount();
	ASSERT_EQ(edgeCount, 9);

	int degree = testGraph.getNodeDegree(4);
	ASSERT_EQ(degree, 4);

	degree = testGraph.getNodeDegree(5);
	ASSERT_EQ(degree, 4);
}

/*
			  9
			  |
			  6
			  | \
 10---7---1---3---5
	\ |\ /| \ |\ /|
	  8---0---2---4
*/
TEST_F(CColorableGraphUnitTest, FunctionColorWithSLO)
{
	CColorableGraphPtr Graph = CColorableGraphPtr(new CColorableGraph(11));
	Graph->addEdgeV(0, 1);
	Graph->addEdgeV(0, 2);
	Graph->addEdgeV(1, 2);
	Graph->addEdgeV(1, 3);
	Graph->addEdgeV(2, 3);
	Graph->addEdgeV(2, 4);
	Graph->addEdgeV(2, 5);
	Graph->addEdgeV(3, 4);
	Graph->addEdgeV(3, 5);
	Graph->addEdgeV(3, 6);
	Graph->addEdgeV(4, 5);
	Graph->addEdgeV(5, 6);
	Graph->addEdgeV(7, 1);
	Graph->addEdgeV(0, 8);
	Graph->addEdgeV(8, 7);
	Graph->addEdgeV(7, 0);
	Graph->addEdgeV(8, 1);
	Graph->addEdgeV(6, 9);
	Graph->addEdgeV(10, 7);
	Graph->addEdgeV(10, 8);

	Graph->setColorNumberQ(2);
	Graph->searchMaximalCliquesByBronKerbosch();
	std::vector<std::vector<int>> Result = Graph->colorGraphWithSLO();
	ASSERT_EQ(Result.size(), 2);
}

/*
			  9
			  |
			  6
			  | \
 10---7---1---3---5
    \ |\ /| \ |\ /|
	  8---0---2---4
*/
TEST_F(CColorableGraphUnitTest, FunctionColorWithClique)
{
	CColorableGraphPtr Graph = CColorableGraphPtr(new CColorableGraph(11));
	Graph->addEdgeV(0, 1);
	Graph->addEdgeV(0, 2);
	Graph->addEdgeV(1, 2);
	Graph->addEdgeV(1, 3);
	Graph->addEdgeV(2, 3);
	Graph->addEdgeV(2, 4);
	Graph->addEdgeV(2, 5);
	Graph->addEdgeV(3, 4);
	Graph->addEdgeV(3, 5);
	Graph->addEdgeV(3, 6);
	Graph->addEdgeV(4, 5);
	Graph->addEdgeV(5, 6);
	Graph->addEdgeV(7, 1);
	Graph->addEdgeV(0, 8);
	Graph->addEdgeV(8, 7);
	Graph->addEdgeV(7, 0);
	Graph->addEdgeV(8, 1);
	Graph->addEdgeV(6, 9);
	Graph->addEdgeV(10, 7);
	Graph->addEdgeV(10, 8);

	Graph->setColorNumberQ(2);
	Graph->searchMaximalCliquesByBronKerbosch();
	std::vector<std::vector<int>> Result = Graph->colorGraphWithClique();
	ASSERT_EQ(Result.size(), 2);
}

/*
		  6

	  1   3---5
	  |   |\/ | \
	  0   2---4---7
*/

TEST_F(CColorableGraphUnitTest, FunctionColorWithClique2)
{
	CColorableGraphPtr Graph = CColorableGraphPtr(new CColorableGraph(8));
	Graph->addEdgeV(0, 1);
	Graph->addEdgeV(2, 3);
	Graph->addEdgeV(2, 4);
	Graph->addEdgeV(2, 5);
	Graph->addEdgeV(3, 4);
	Graph->addEdgeV(3, 5);
	Graph->addEdgeV(4, 5);
	Graph->addEdgeV(4, 7);
	Graph->addEdgeV(5, 7);

	Graph->setColorNumberQ(2);
	Graph->searchMaximalCliquesByBronKerbosch();
	std::vector<std::vector<int>> Result = Graph->colorGraphWithClique();
	ASSERT_EQ(Result.size(), 2);
}

/*
			  9
			  |
			  6
			  | \
 10---7---1---3---5
	\ |\ /| \ |\ /|
	  8---0---2---4
*/
TEST_F(CColorableGraphUnitTest, FunctionColorWithEdge)
{
	CColorableGraphPtr Graph = CColorableGraphPtr(new CColorableGraph(11));
	Graph->addEdgeV(0, 1);
	Graph->addEdgeV(0, 2);
	Graph->addEdgeV(1, 2);
	Graph->addEdgeV(1, 3);
	Graph->addEdgeV(2, 3);
	Graph->addEdgeV(2, 4);
	Graph->addEdgeV(2, 5);
	Graph->addEdgeV(3, 4);
	Graph->addEdgeV(3, 5);
	Graph->addEdgeV(3, 6);
	Graph->addEdgeV(4, 5);
	Graph->addEdgeV(5, 6);
	Graph->addEdgeV(7, 1);
	Graph->addEdgeV(0, 8);
	Graph->addEdgeV(8, 7);
	Graph->addEdgeV(7, 0);
	Graph->addEdgeV(8, 1);
	Graph->addEdgeV(6, 9);
	Graph->addEdgeV(10, 7);
	Graph->addEdgeV(10, 8);

	Graph->setColorNumberQ(2);
	Graph->searchMaximalCliquesByBronKerbosch();
	std::vector<std::vector<int>> Result = Graph->colorGraphWithEdge();
	ASSERT_EQ(Result.size(), 2);
}

/*
		  6

	  1   3---5
	  |   |\/ | \
	  0   2---4---7
*/

TEST_F(CColorableGraphUnitTest, FunctionColorWithEdge2)
{
	CColorableGraphPtr Graph = CColorableGraphPtr(new CColorableGraph(8));
	Graph->addEdgeV(0, 1);
	Graph->addEdgeV(2, 3);
	Graph->addEdgeV(2, 4);
	Graph->addEdgeV(2, 5);
	Graph->addEdgeV(3, 4);
	Graph->addEdgeV(3, 5);
	Graph->addEdgeV(4, 5);
	Graph->addEdgeV(4, 7);
	Graph->addEdgeV(5, 7);

	Graph->setColorNumberQ(2);
	Graph->searchMaximalCliquesByBronKerbosch();
	std::vector<std::vector<int>> Result = Graph->colorGraphWithEdge();
	ASSERT_EQ(Result.size(), 2);
}