import matplotlib.pyplot as plt

def draw(Group, subplot, subplotX, subPlotY):
    GroupData = []
    for line in Group:
        dataline = line.split(" ")
        Temp = len(dataline)-1
        GroupData.append(Temp)

    subplot[subPlotY].bar(range(len(GroupData)), GroupData, fc='b')


DistanceGroup = open("./Group0.txt", 'r')
VolumeGroup = open("./Group1.txt", 'r')
ShapeMatchingGroup = open("./Group2.txt", 'r')

fig, ax = plt.subplots(1,3)
draw(DistanceGroup,ax,0,0)
draw(VolumeGroup,ax,0,1)
draw(ShapeMatchingGroup,ax,0,2)

plt.show()

DistanceGroup.close()
VolumeGroup.close()
ShapeMatchingGroup.close()