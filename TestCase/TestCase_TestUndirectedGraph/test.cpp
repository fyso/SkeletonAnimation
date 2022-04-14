#include "pch.h"
#include "UndirectedGraph.h"

using namespace Animation;

/*
0----1   2----3
*/
TEST(TestUndirectedGraph, TestFindShortestDistanceWithSeparatedNode)
{
	CUndirectedGraph Graph(4);
	Graph.addEdge(0, 1);
	Graph.addEdge(2, 3);

	EXPECT_EQ(MAX_DISTANCE, Graph.findShortestDistanceByDijskra(0, 3));
	EXPECT_EQ(1, Graph.findShortestDistanceByDijskra(2, 3));
	Graph.generateShortestDistanceOf(0);
	EXPECT_EQ(1, Graph.findShortestDistance(0, 1));
	EXPECT_THROW(Graph.findShortestDistance(0, 2), std::runtime_error);
	EXPECT_THROW(Graph.findShortestDistance(0, 3), std::runtime_error);
	EXPECT_THROW(Graph.findShortestDistance(1, 3), std::runtime_error);

	std::vector<int> Indices = { 0, 1, 2, 3 };
	std::vector<int> ExpectedDistanceSet = { 0, 1, MAX_DISTANCE, MAX_DISTANCE };
	std::vector<int> DistanceSet = Graph.findShortestDistanceByDijskra(0, Indices);
	EXPECT_EQ(ExpectedDistanceSet.size(), DistanceSet.size());
	for (int i = 0; i < DistanceSet.size(); i++)
	{
		EXPECT_EQ(ExpectedDistanceSet[i], DistanceSet[i]);
	}
}

/*
0----1----2----3
*/
TEST(TestUndirectedGraph, TestFindShortestDistanceWithFourNode)
{
	CUndirectedGraph Graph(4);
	Graph.addEdge(0, 1);
	Graph.addEdge(1, 2);
	Graph.addEdge(2, 3);

	EXPECT_EQ(3, Graph.findShortestDistanceByDijskra(0, 3));
	EXPECT_EQ(2, Graph.findShortestDistanceByDijskra(1, 3));
	EXPECT_THROW(Graph.findShortestDistance(1, 3), std::runtime_error);

	std::vector<int> Indices = { 1, 2, 3 };
	std::vector<int> ExpectedDistanceSet = { 1, 2, 3 };
	std::vector<int> DistanceSet = Graph.findShortestDistanceByDijskra(0, Indices);
	Graph.generateShortestDistanceOf(0);
	std::vector<int> DistanceSet1 = Graph.findShortestDistance(0, Indices);
	EXPECT_EQ(ExpectedDistanceSet.size(), DistanceSet.size());
	EXPECT_EQ(ExpectedDistanceSet.size(), DistanceSet1.size());
	for (int i = 0; i < DistanceSet.size(); i++)
	{
		EXPECT_EQ(ExpectedDistanceSet[i], DistanceSet[i]);
		EXPECT_EQ(ExpectedDistanceSet[i], DistanceSet1[i]);
	}
}

/*
0----1----3----4
  \           /
	----2----
*/
TEST(TestUndirectedGraph, TestFindShortestDistanceWithFiveNode)
{
	CUndirectedGraph Graph(5);
	Graph.addEdge(0, 1);
	Graph.addEdge(1, 3);
	Graph.addEdge(0, 2);
	Graph.addEdge(2, 4);
	Graph.addEdge(3, 4);

	EXPECT_EQ(2, Graph.findShortestDistanceByDijskra(0, 3));
	EXPECT_THROW(Graph.findShortestDistance(0, 3), std::runtime_error);

	std::vector<int> Indices = { 1, 2, 3, 4 };
	std::vector<int> ExpectedDistanceSet = { 1, 1, 2, 2 };
	std::vector<int> DistanceSet = Graph.findShortestDistanceByDijskra(0, Indices);
	Graph.generateShortestDistanceOf(0);
	std::vector<int> DistanceSet1 = Graph.findShortestDistance(0, Indices);
	EXPECT_EQ(ExpectedDistanceSet.size(), DistanceSet.size());
	EXPECT_EQ(ExpectedDistanceSet.size(), DistanceSet1.size());
	for (int i = 0; i < DistanceSet.size(); i++)
	{
		EXPECT_EQ(ExpectedDistanceSet[i], DistanceSet[i]);
		EXPECT_EQ(ExpectedDistanceSet[i], DistanceSet1[i]);
	}
}

/*
	5----4----3
	| \     / |
	|    2    |
	| /     \ |
	0 --------1
*/
TEST(TestUndirectedGraph, TestFindShortestDistanceWithSixNode)
{
	CUndirectedGraph Graph(6);
	Graph.addEdge(0, 1);
	Graph.addEdge(0, 2);
	Graph.addEdge(0, 5);
	Graph.addEdge(1, 2);
	Graph.addEdge(1, 3);
	Graph.addEdge(2, 3);
	Graph.addEdge(2, 5);
	Graph.addEdge(3, 4);
	Graph.addEdge(4, 5);

	EXPECT_EQ(2, Graph.findShortestDistanceByDijskra(4, 0));
	EXPECT_EQ(2, Graph.findShortestDistanceByDijskra(1, 5));
	EXPECT_THROW(Graph.findShortestDistance(0, 3), std::runtime_error);

	std::vector<int> Indices = { 0, 1, 2, 3, 4, 5 };
	std::vector<int> ExpectedDistanceSet = { 2, 2, 2, 1, 0, 1 };
	std::vector<int> DistanceSet = Graph.findShortestDistanceByDijskra(4, Indices);
	Graph.generateShortestDistanceOf(4);
	std::vector<int> DistanceSet1 = Graph.findShortestDistance(4, Indices);
	EXPECT_EQ(ExpectedDistanceSet.size(), DistanceSet.size());
	EXPECT_EQ(ExpectedDistanceSet.size(), DistanceSet1.size());
	for (int i = 0; i < DistanceSet.size(); i++)
	{
		EXPECT_EQ(ExpectedDistanceSet[i], DistanceSet[i]);
		EXPECT_EQ(ExpectedDistanceSet[i], DistanceSet1[i]);
	}
}

/*
		  6
		  |  \
	  1---3---5
	  | \ | \ |
	  0---2---4
*/
TEST(TestUndirectedGraph, TestFindShortestDistanceWithSevenNode)
{
	CUndirectedGraph Graph(7);
	Graph.addEdge(0, 1);
	Graph.addEdge(0, 2);
	Graph.addEdge(1, 2);
	Graph.addEdge(1, 3);
	Graph.addEdge(2, 3);
	Graph.addEdge(2, 4);
	Graph.addEdge(3, 4);
	Graph.addEdge(3, 5);
	Graph.addEdge(3, 6);
	Graph.addEdge(4, 5);
	Graph.addEdge(5, 6);

	EXPECT_EQ(2, Graph.findShortestDistanceByDijskra(4, 0));
	EXPECT_EQ(3, Graph.findShortestDistanceByDijskra(0, 6));
	EXPECT_THROW(Graph.findShortestDistance(0, 3), std::runtime_error);

	std::vector<int> Indices = { 0, 1, 2, 3, 4, 5 };
	std::vector<int> ExpectedDistanceSet = { 3, 2, 2, 1, 2, 1 };
	std::vector<int> DistanceSet = Graph.findShortestDistanceByDijskra(6, Indices);
	Graph.generateShortestDistanceOf(6);
	std::vector<int> DistanceSet1 = Graph.findShortestDistance(6, Indices);
	EXPECT_EQ(ExpectedDistanceSet.size(), DistanceSet.size());
	EXPECT_EQ(ExpectedDistanceSet.size(), DistanceSet1.size());
	for (int i = 0; i < DistanceSet.size(); i++)
	{
		EXPECT_EQ(ExpectedDistanceSet[i], DistanceSet[i]);
		EXPECT_EQ(ExpectedDistanceSet[i], DistanceSet1[i]);
	}
}

/*
	 0		 4 -- 6
	   \   /   \
	1 -- 3		 7
	   /   \
	 2		 5
*/
TEST(TestUndirectedGraph, TestFindShortestDistanceWithEightNode)
{
	CUndirectedGraph Graph(8);
	Graph.addEdge(0, 3);
	Graph.addEdge(1, 3);
	Graph.addEdge(2, 3);
	Graph.addEdge(3, 4);
	Graph.addEdge(3, 5);
	Graph.addEdge(4, 6);
	Graph.addEdge(4, 7);

	EXPECT_EQ(3, Graph.findShortestDistanceByDijskra(0, 6));
	EXPECT_EQ(3, Graph.findShortestDistanceByDijskra(5, 7));
	EXPECT_THROW(Graph.findShortestDistance(0, 4), std::runtime_error);

	std::vector<int> Indices1 = { 0, 1, 2, 3, 4, 5, 6, 7 };
	std::vector<int> ExpectedDistanceSet1 = { 1, 1, 1, 0, 1, 1, 2, 2 };
	std::vector<int> DistanceSet1 = Graph.findShortestDistanceByDijskra(3, Indices1);
	Graph.generateShortestDistanceOf(3);
	std::vector<int> DistanceSet11 = Graph.findShortestDistance(3, Indices1);
	EXPECT_EQ(ExpectedDistanceSet1.size(), DistanceSet1.size());
	EXPECT_EQ(ExpectedDistanceSet1.size(), DistanceSet11.size());
	for (int i = 0; i < DistanceSet1.size(); i++)
	{
		EXPECT_EQ(ExpectedDistanceSet1[i], DistanceSet1[i]);
		EXPECT_EQ(ExpectedDistanceSet1[i], DistanceSet11[i]);
	}

	std::vector<int> Indices2 = { 0, 1, 2, 3, 4, 5, 6, 7 };
	std::vector<int> ExpectedDistanceSet2 = { 3, 3, 3, 2, 1, 3, 0, 2 };
	std::vector<int> DistanceSet2 = Graph.findShortestDistanceByDijskra(6, Indices2);
	Graph.generateShortestDistanceOf(6);
	std::vector<int> DistanceSet21 = Graph.findShortestDistance(6, Indices2);
	EXPECT_EQ(ExpectedDistanceSet2.size(), DistanceSet2.size());
	EXPECT_EQ(ExpectedDistanceSet2.size(), DistanceSet21.size());
	for (int i = 0; i < DistanceSet2.size(); i++)
	{
		EXPECT_EQ(ExpectedDistanceSet2[i], DistanceSet2[i]);
		EXPECT_EQ(ExpectedDistanceSet2[i], DistanceSet21[i]);
	}
}

/*
	 0	---	 4 -- 6
	   \   /   \
	1 -- 3	---	 7
	   /   \
	 2	---	 5
*/
TEST(TestUndirectedGraph, TestLargestDegreeFirstColorGraph)
{
	CUndirectedGraph Graph(8);
	Graph.addEdge(0, 3);
	Graph.addEdge(0, 4);
	Graph.addEdge(1, 3);
	Graph.addEdge(2, 3);
	Graph.addEdge(2, 5);
	Graph.addEdge(3, 4);
	Graph.addEdge(3, 5);
	Graph.addEdge(3, 7);
	Graph.addEdge(4, 6);
	Graph.addEdge(4, 7);

	std::vector<std::vector<int>> colorResult = Graph.doWelchPowellColorGraph();
	EXPECT_EQ(colorResult.size(), 3);
	EXPECT_EQ(colorResult[0].size(), 2);
	EXPECT_EQ(colorResult[1].size(), 3);
	EXPECT_EQ(colorResult[2].size(), 3);
}

/*
		  6
		  |  \
	  1---3---5
	  | \ |\ /|
	  0---2---4
*/
TEST(TestUndirectedGraph, TestSmallestLastOrderColorGraph)
{
	CUndirectedGraph Graph(7);
	Graph.addEdge(0, 1);
	Graph.addEdge(0, 2);
	Graph.addEdge(1, 2);
	Graph.addEdge(1, 3);
	Graph.addEdge(2, 3);
	Graph.addEdge(2, 4);
	Graph.addEdge(2, 5);
	Graph.addEdge(3, 4);
	Graph.addEdge(3, 5);
	Graph.addEdge(3, 6);
	Graph.addEdge(4, 5);
	Graph.addEdge(5, 6);

	std::vector<std::vector<int>> colorResult = Graph.doSLOColorGraph();
	EXPECT_EQ(colorResult.size(), 4);
	EXPECT_EQ(colorResult[0].size(), 2);
	EXPECT_EQ(colorResult[1].size(), 2);
	EXPECT_EQ(colorResult[2].size(), 2);
	EXPECT_EQ(colorResult[3].size(), 1);
}

/*
		  6
		  |  \
	  1---3---5
	  | \ |\ /|
	  0---2---4
*/
TEST(TestUndirectedGraph, TestColorFunction)
{
	CUndirectedGraph Graph(7);
	Graph.addEdge(0, 1);
	Graph.addEdge(0, 2);
	Graph.addEdge(1, 2);
	Graph.addEdge(1, 3);
	Graph.addEdge(2, 3);
	Graph.addEdge(2, 4);
	Graph.addEdge(2, 5);
	Graph.addEdge(3, 4);
	Graph.addEdge(3, 5);
	Graph.addEdge(3, 6);
	Graph.addEdge(4, 5);
	Graph.addEdge(5, 6);

	Graph.setColorNumberQ(2);
	Graph.generateAllMaximalCliquesByBronKerbosch();
	std::vector<std::vector<int>> AllCliques = Graph.getMaximalCliques();

	for (int i = 0; i < AllCliques.size(); i++)
	{
		for (int t = 0; t < AllCliques[i].size(); t++)
		{
			std::cout << i << ":" << AllCliques[i][t] << std::endl;
		}
	}
	EXPECT_EQ(1, 1);
}