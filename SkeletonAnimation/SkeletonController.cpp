#include "SkeletonController.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <iostream>
#include <common/FileSystem.h>

Animation::CSkeletonController::CSkeletonController(const std::string& vFilePath)
{
	if (vFilePath.empty())
		throw std::invalid_argument("The file path is empty.");

	if (!initSkeletonByFile(vFilePath))
		throw std::runtime_error("Failed to initialize skeleton.");
}

//*********************************************************************
//FUNCTION:
bool Animation::CSkeletonController::initSkeletonByFile(const std::string& vFilePath)
{
	if (!hiveUtility::hiveFileSystem::hiveIsFileExisted(vFilePath))
	{
		std::cout << "The skeleton file doesn't exist." << std::endl;
		return false;
	}

	std::ifstream File(vFilePath);
	if (!File.is_open())
	{
		std::cout << "Can't open the skeleton file." << std::endl;
		return false;
	}

	std::vector<std::string> AllFileStr;
	std::string Line;
	while (getline(File, Line)) AllFileStr.push_back(Line);
	File.close();

	try
	{
		__generateSkeleton(AllFileStr);

		__generateAllPose(AllFileStr);
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return false;
	}

	__generateInitialData();

	return true;
}

//*********************************************************************
//FUNCTION:
bool Animation::CSkeletonController::dumpNextPose(std::vector<Eigen::Matrix4d>& voSkeletonTransfer)
{
	if (m_AllPose.size() == 0)
	{
		std::cout << "The skeleton haven't been initialized." << std::endl;
		return false;
	}

	m_CurrentPoseIndex++;
	if (m_CurrentPoseIndex >= m_AllPose.size())
	{
		return false;
	}

	voSkeletonTransfer = m_AllPose[m_CurrentPoseIndex];
	return true;
}

//*********************************************************************
//FUNCTION:
void Animation::CSkeletonController::resetPose()
{
	m_CurrentPoseIndex = -1;
}

//*********************************************************************
//FUNCTION:
int Animation::CSkeletonController::getParentIndexAt(int vIndex) const
{
	if (vIndex < 0 || vIndex >= m_Skeleton.size())
		throw std::invalid_argument("Index is out of range.");

	return m_Skeleton[vIndex].ParentIndex;
}

//*********************************************************************
//FUNCTION:
const Eigen::Vector3d Animation::CSkeletonController::getInitialBoneWorldPos(int vIndex) const
{
	if (vIndex < 0 || vIndex >= m_InitialWorldPos.size())
		throw std::out_of_range("Index is out of range.");

	return m_InitialWorldPos[vIndex];
}

//*********************************************************************
//FUNCTION:
const Eigen::Matrix4d Animation::CSkeletonController::getWorldToLocalTransfer(int vIndex) const
{
	if (vIndex < 0 || vIndex >= m_WorldToLocalTransfer.size())
		throw std::invalid_argument("Index is out of range.");

	return m_WorldToLocalTransfer[vIndex];
}

//*********************************************************************
//FUNCTION:
void Animation::CSkeletonController::__generateSkeleton(const std::vector<std::string>& vAllFileStr)
{
	m_Skeleton.clear();
	for (auto Str : vAllFileStr)
	{
		std::vector<std::string> LineStr;
		boost::split(LineStr, Str, boost::is_any_of(","));
		if (LineStr[0] == "BONEEND")
		{
			break;
		}
		else
		{
			SSkeletonDefinition Skeleton;
			Skeleton.ParentIndex = boost::lexical_cast<int>(LineStr[0]);
			Skeleton.BoneID = boost::lexical_cast<int>(LineStr[1]);
			float X = boost::lexical_cast<double>(LineStr[2]);
			float Y = boost::lexical_cast<double>(LineStr[3]);
			float Z = boost::lexical_cast<double>(LineStr[4]);
			Skeleton.Offset = Eigen::Vector3d(X, Y, Z);

			m_Skeleton.push_back(Skeleton);
		}
	}
}

//*********************************************************************
//FUNCTION:
void Animation::CSkeletonController::__generateAllPose(const std::vector<std::string>& vAllFileStr)
{
	const int SkeletonSize = m_Skeleton.size();
	const int StrStart = SkeletonSize + 1;
	const int StrEnd = vAllFileStr.size();
	_ASSERTE((StrEnd - StrStart) % SkeletonSize == 0);
	m_AllPose.resize((StrEnd - StrStart) / SkeletonSize);
#pragma omp parallel for
	for (int i = StrStart; i < StrEnd; i += SkeletonSize)
	{
		std::vector<Eigen::Matrix4d> TempPose;
		for (int k = 0; k < SkeletonSize; k++)
		{
			std::vector<std::string> LineStr;
			boost::split(LineStr, vAllFileStr[i + k], boost::is_any_of(","));
			Eigen::Matrix4d TransferMatrix;
			TransferMatrix(0, 0) = boost::lexical_cast<double>(LineStr[0]);
			TransferMatrix(0, 1) = boost::lexical_cast<double>(LineStr[1]);
			TransferMatrix(0, 2) = boost::lexical_cast<double>(LineStr[2]);
			TransferMatrix(0, 3) = boost::lexical_cast<double>(LineStr[3]);
			TransferMatrix(1, 0) = boost::lexical_cast<double>(LineStr[4]);
			TransferMatrix(1, 1) = boost::lexical_cast<double>(LineStr[5]);
			TransferMatrix(1, 2) = boost::lexical_cast<double>(LineStr[6]);
			TransferMatrix(1, 3) = boost::lexical_cast<double>(LineStr[7]);
			TransferMatrix(2, 0) = boost::lexical_cast<double>(LineStr[8]);
			TransferMatrix(2, 1) = boost::lexical_cast<double>(LineStr[9]);
			TransferMatrix(2, 2) = boost::lexical_cast<double>(LineStr[10]);
			TransferMatrix(2, 3) = boost::lexical_cast<double>(LineStr[11]);
			TransferMatrix(3, 0) = boost::lexical_cast<double>(LineStr[12]);
			TransferMatrix(3, 1) = boost::lexical_cast<double>(LineStr[13]);
			TransferMatrix(3, 2) = boost::lexical_cast<double>(LineStr[14]);
			TransferMatrix(3, 3) = boost::lexical_cast<double>(LineStr[15]);

			TempPose.push_back(TransferMatrix);
		}
		m_AllPose[(i - StrStart) / SkeletonSize] = TempPose;
	}
}

//*********************************************************************
//FUNCTION:
void Animation::CSkeletonController::__generateInitialData()
{
	const int SkeletonSize = m_Skeleton.size();
	std::vector<Eigen::Matrix4d> InitialTransferMatrix = m_AllPose[0];
	_ASSERTE(SkeletonSize == InitialTransferMatrix.size());

	for (int i = 0; i < SkeletonSize; i++)
	{
		Eigen::Vector3d WorldPos = Eigen::Vector3d(InitialTransferMatrix[i](0, 3), InitialTransferMatrix[i](1, 3), InitialTransferMatrix[i](2, 3));
		m_InitialWorldPos.push_back(WorldPos);
		m_WorldToLocalTransfer.push_back(InitialTransferMatrix[i].inverse());
	}
}