import numpy as np
import matplotlib.pyplot as plt
import matplotlib.collections as collections

def draw(Graph, Group, subplot, subplotX, subPlotY):
    Graph.seek(0)
    Group.seek(0)
    X = []
    Y = []
    if subplotX == 2:
        GroupData = []
        GraphData = {int:[]}
        for line in Group:
            Temp = []
            dataline = line.split(" ")
            for node in dataline:
                if node == ';\n':
                    continue
                Temp.append(int(node))
            GroupData.append(Temp)

        for line in Graph:
            currentNode = -1
            dataline = line.split(" ")
            for s in dataline:
                if s == ';\n':
                    continue
                if currentNode == -1:
                    currentNode = int(s)
                    GraphData[currentNode] = []
                    continue
                Index = int(s)
                GraphData[currentNode].append(Index)

        indexX = 0
        for g1 in GroupData:
            for index1 in g1:
                indexY = 0
                for g2 in GroupData:
                    for index2 in g2:
                        if GraphData[index1].count(index2) > 0:
                            X.append(indexX)
                            Y.append(indexY)
                        indexY += 1
                indexX += 1  
    else:
        for line in Graph:
            currentNode = -1
            dataline = line.split(" ")
            for s in dataline:
                if s == ';\n':
                    continue
                if currentNode == -1:
                    currentNode = int(s)
                    continue
                Index = int(s)
                X.append(currentNode)
                Y.append(Index)

    subplot[subplotX, subPlotY].scatter(X,Y, s=0.1, alpha=0.5)
        

DistanceOrginFile = open("./OrginGraph0.txt", 'r')
DistanceProcessedFile = open("./ProcessedGraph0.txt", 'r')
VolumeOrginFile = open("./OrginGraph1.txt", 'r')
VolumeProcessedFile = open("./ProcessedGraph1.txt", 'r')
ShapeMatchingOrginFile = open("./OrginGraph2.txt", 'r')
ShapeMatchingProcessedFile = open("./ProcessedGraph2.txt", 'r')
DistanceGroup = open("./Group0.txt", 'r')
VolumeGroup = open("./Group1.txt", 'r')
ShapeMatchingGroup = open("./Group2.txt", 'r')

fig, ax = plt.subplots(3,3)
#距离约束
draw(DistanceOrginFile,DistanceGroup,ax,0,0)
draw(DistanceProcessedFile,DistanceGroup,ax,1,0)
draw(DistanceProcessedFile,DistanceGroup,ax,2,0)
#体积约束
draw(VolumeOrginFile,VolumeGroup,ax,0,1)
draw(VolumeProcessedFile,VolumeGroup,ax,1,1)
draw(VolumeProcessedFile,VolumeGroup,ax,2,1)
#形状匹配约束
draw(ShapeMatchingOrginFile,ShapeMatchingGroup,ax,0,2)
draw(ShapeMatchingProcessedFile,ShapeMatchingGroup,ax,1,2)
draw(ShapeMatchingProcessedFile,ShapeMatchingGroup,ax,2,2)
plt.show()

#清理文件
DistanceOrginFile.close()
DistanceProcessedFile.close()
VolumeOrginFile.close()
VolumeProcessedFile.close()
ShapeMatchingOrginFile.close()
ShapeMatchingProcessedFile.close()
DistanceGroup.close()
VolumeGroup.close()
ShapeMatchingGroup.close()