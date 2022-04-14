#include "Voxelizer.h"
#include "AnimationConfig.h"
#include <boost/lexical_cast.hpp>
#include <glm/ext.hpp>

constexpr auto TOTAL_VOXEL_SIZE_LINE = 0;
constexpr auto FILLED_VOXEL_SIZE_LINE = 1;
constexpr auto START_POSITION_LINE = 2;
constexpr auto SPACING_LINE = 3;
constexpr auto VOXEL_INDEX_LINE = 4;

//*********************************************************************
//FUNCTION:
void Animation::CVoxelizer::voxelize(const std::string& vModelFilePath)
{
	if (vModelFilePath.empty()) throw std::invalid_argument("Can't find model file.");

	__init(vModelFilePath);

	__voxelizeModelSurface();
	__voxelizeModelInterior();

	//std::cout << "Surface+Interior: " << m_VoxelStatusSet.count() << std::endl;
	//std::cout << "Total: " << m_VoxelStatusSet.size() << std::endl;
}

//*********************************************************************
//FUNCTION:
void Animation::CVoxelizer::sendVoxels(CParticleSource& vParticleSource)
{
	SVoxelizationInfo VoxelizationInfo;

	VoxelizationInfo.TotalVoxelSize = Eigen::Vector3i(m_VoxelResolution.x, m_VoxelResolution.y, m_VoxelResolution.z);

	int MinX = m_VoxelResolution.x, MinY = m_VoxelResolution.y, MinZ = m_VoxelResolution.z;
	int MaxX = 0, MaxY = 0, MaxZ = 0;
	for (int x = 0; x < m_VoxelResolution.x; x++)
	{
		for (int y = 0; y < m_VoxelResolution.y; y++)
		{
			for (int z = 0; z < m_VoxelResolution.z; z++)
			{
				const int Index = __transformIndex2Int(glm::ivec3(x, y, z));
				_ASSERTE(Index >= 0 && Index < m_VoxelStatusSet.size());
				if (m_VoxelStatusSet[Index])
				{
					if (x < MinX) MinX = x;
					if (x > MaxX) MaxX = x;
					if (y < MinY) MinY = y;
					if (y > MaxY) MaxY = y;
					if (z < MinZ) MinZ = z;
					if (z > MaxZ) MaxZ = z;
				}
			}
		}
	}
	VoxelizationInfo.FilledVoxelSize = Eigen::Vector3i(MaxX - MinX, MaxY - MinY, MaxZ - MinZ);

	double HalfVoxelSize = m_VoxelSize / 2;
	VoxelizationInfo.StartPos = Eigen::Vector3d(m_ModelAABB.MinCoordinate.x + HalfVoxelSize, m_ModelAABB.MinCoordinate.y + HalfVoxelSize, m_ModelAABB.MinCoordinate.z + HalfVoxelSize);
	VoxelizationInfo.Size = m_VoxelSize;
	vParticleSource.setVoxelizationInfo(VoxelizationInfo);

	for (int x = 0; x < m_VoxelResolution.x; x++)
		for (int y = 0; y < m_VoxelResolution.y; y++)
			for (int z = 0; z < m_VoxelResolution.z; z++)
			{
				const int Index = __transformIndex2Int(glm::ivec3(x, y, z));
				_ASSERTE(Index >= 0 && Index < m_VoxelStatusSet.size());
				if (m_VoxelStatusSet[Index])
				{
					const int SplitingCount = x + y + z;
					Eigen::Vector3i VoxelLocation = Eigen::Vector3i(x, y, z);
					double PosX = VoxelizationInfo.StartPos[0] + x * VoxelizationInfo.Size;
					double PosY = VoxelizationInfo.StartPos[1] + y * VoxelizationInfo.Size;
					double PosZ = VoxelizationInfo.StartPos[2] + z * VoxelizationInfo.Size;
					Eigen::Vector3d VoxelCenterPos = Eigen::Vector3d(PosX, PosY, PosZ);
					
					vParticleSource.addVoxel(SplitingCount, VoxelLocation, VoxelCenterPos);
				}
			}
}

bool Animation::CVoxelizer::loadVoxelData(const std::string& vVoxelDataFilePath)
{
	std::ifstream File(vVoxelDataFilePath);
	if (!File.is_open())
	{
		std::cout << "Can't open the voxel data file." << std::endl;
		return false;
	}
	std::vector<std::string> AllStrFromFile;
	std::string Line;
	while (std::getline(File, Line))  AllStrFromFile.push_back(Line);
	File.close();

	if (AllStrFromFile.size() <= VOXEL_INDEX_LINE)
	{
		std::cout << "There is not enough data in the file." << std::endl;
		return false;
	}

	__generateVoxelsInfo(AllStrFromFile);

	//FIXME: 此处可以先将成员属性的push_back改为赋值方法，然后使用OpenMP加速
	int StrSize = AllStrFromFile.size() - VOXEL_INDEX_LINE;

	m_VoxelStatusSet.resize(m_VoxelResolution.x * m_VoxelResolution.y * m_VoxelResolution.z, false);
#pragma omp parallel for
	for (int i = 0; i < StrSize; ++i)
	{
		std::vector<std::string> CurrentLineSplited;
		boost::split(CurrentLineSplited, AllStrFromFile[i + VOXEL_INDEX_LINE], boost::is_any_of(","));

		auto Tmp_x = boost::lexical_cast<int>(CurrentLineSplited[1]);
		auto Tmp_y = boost::lexical_cast<int>(CurrentLineSplited[2]);
		auto Tmp_z = boost::lexical_cast<int>(CurrentLineSplited[3]);

		int Index = __transformIndex2Int(glm::vec3(Tmp_x, Tmp_y, Tmp_z));
		m_VoxelStatusSet[Index] = true;

	}

}

void Animation::CVoxelizer::saveVoxelData(const std::string & vVoxelDataFilePath)
{
	std::fstream VoxelFile(vVoxelDataFilePath, std::fstream::out);
	_ASSERT(VoxelFile.is_open());

	VoxelFile << m_VoxelResolution.x << "," << m_VoxelResolution.y << "," << m_VoxelResolution.z << std::endl;

	int MinX = m_VoxelResolution.x, MinY = m_VoxelResolution.y, MinZ = m_VoxelResolution.z;
	int MaxX = 0, MaxY = 0, MaxZ = 0;
	for (int x = 0; x < m_VoxelResolution.x; x++)
	{
		for (int y = 0; y < m_VoxelResolution.y; y++)
		{
			for (int z = 0; z < m_VoxelResolution.z; z++)
			{
				const int Index = __transformIndex2Int(glm::ivec3(x, y, z));
				_ASSERTE(Index >= 0 && Index < m_VoxelStatusSet.size());
				if (m_VoxelStatusSet[Index])
				{
					if (x < MinX) MinX = x;
					if (x > MaxX) MaxX = x;
					if (y < MinY) MinY = y;
					if (y > MaxY) MaxY = y;
					if (z < MinZ) MinZ = z;
					if (z > MaxZ) MaxZ = z;
				}
			}
		}
	}
	VoxelFile << MaxX - MinX << "," << MaxY - MinY << "," << MaxZ - MinZ << std::endl;

	double HalfVoxelSize = m_VoxelSize / 2;
	VoxelFile << m_ModelAABB.MinCoordinate.x << "," << m_ModelAABB.MinCoordinate.y + HalfVoxelSize << "," << m_ModelAABB.MinCoordinate.z + HalfVoxelSize << std::endl;
	VoxelFile <<m_VoxelSize << std::endl;

	for (int x = 0; x < m_VoxelResolution.x; x++)
	{
		for (int y = 0; y < m_VoxelResolution.y; y++)
		{
			for (int z = 0; z < m_VoxelResolution.z; z++)
			{
				int Index = __transformIndex2Int(glm::vec3(x, y, z));
				if (m_VoxelStatusSet[Index])
				{
					VoxelFile << x+y+z << "," << x << "," << y << "," << z << std::endl;
				}
			}
		}
	}

	VoxelFile.close();
}

//*********************************************************************
//FUNCTION:
void Animation::CVoxelizer::__init(const std::string& vModelFilePath)
{
	_ASSERTE(!vModelFilePath.empty());

	m_pModel = boost::shared_ptr<graphics::CModel>(new graphics::CModel(vModelFilePath));
	if (m_pModel == nullptr)
		throw std::runtime_error("Failed to load model.");
	
	m_VoxelSize = CAnimationConfig::getInstance()->getAttribute<float>(KEY_WORDS::VOXEL_SIZE);
	_ASSERTE(m_VoxelSize > 0);
	m_AABBPadding = m_VoxelSize * CAnimationConfig::getInstance()->getAttribute<float>(KEY_WORDS::AABB_PADDING_RATIO);
	__calculateModelAABB();

	m_VoxelResolution = glm::round((m_ModelAABB.MaxCoordinate - m_ModelAABB.MinCoordinate) / m_VoxelSize);
	m_VoxelStatusSet.resize(m_VoxelResolution.x * m_VoxelResolution.y * m_VoxelResolution.z, false);
}

//*********************************************************************
//FUNCTION:
void Animation::CVoxelizer::__calculateModelAABB()
{
	const int MeshesSize = m_pModel->getMumOfMeshes();
	for (int i = 0; i < MeshesSize; i++)
	{
		const graphics::CMesh& Mesh = m_pModel->getMeshesAt(i);
		const int VerticesSize = Mesh.getNumOfVertices();
		for (int k = 0; k < VerticesSize; k++)
		{
			graphics::SVertex Vertex = Mesh.getVertexAt(k);
			m_ModelAABB.updateAABB(Vertex);
		}
	}

	m_ModelAABB.MinCoordinate -= glm::vec3(m_AABBPadding);
	m_ModelAABB.MaxCoordinate += glm::vec3(m_AABBPadding);
}

//*********************************************************************
//FUNCTION:
void Animation::CVoxelizer::__voxelizeModelSurface()
{
	const int VoxelSetLength = m_VoxelResolution.x * m_VoxelResolution.y * m_VoxelResolution.z;
	
	const auto VoxelizeVertexShaderFile = CAnimationConfig::getInstance()->getAttribute<std::string>(KEY_WORDS::VERTEX_SHADER);
	const auto VoxelizeGeometryShaderFile = CAnimationConfig::getInstance()->getAttribute<std::string>(KEY_WORDS::GEOMETRY_SHADER);
	const auto VoxelizeFragmentShaderFile = CAnimationConfig::getInstance()->getAttribute<std::string>(KEY_WORDS::FRAGMENT_SHADER);

	graphics::CShader Shader(VoxelizeVertexShaderFile.c_str(), VoxelizeFragmentShaderFile.c_str(), VoxelizeGeometryShaderFile.c_str());
	Shader.use();

	GLuint CountBuffer;
	__setupCountBuffer(CountBuffer, VoxelSetLength);
	__setupShader(Shader, m_VoxelResolution);

	__tagSurfaceVoxels(Shader, CountBuffer, VoxelSetLength);

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
}

//*********************************************************************
//FUNCTION:
void Animation::CVoxelizer::__voxelizeModelInterior()
{
	_ASSERTE(!m_VoxelStatusSet.empty());

	std::queue<glm::ivec3> VoxelQueue;
	VoxelQueue.push(glm::ivec3(0, 0, 0));
	m_VoxelStatusSet[0] = false;

	__floodVoxelStatusSet(VoxelQueue);
	m_VoxelStatusSet.flip();
}

//*********************************************************************
//FUNCTION:
void Animation::CVoxelizer::__floodVoxelStatusSet(std::queue<glm::ivec3>& vioVoxelQueue)
{
	const std::vector<glm::ivec3> Directions = { {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1} };

	boost::dynamic_bitset<> TempVoxelStatusSet;
	TempVoxelStatusSet.resize(m_VoxelResolution.x * m_VoxelResolution.y * m_VoxelResolution.z, false);

	while (!vioVoxelQueue.empty())
	{
		auto CurrentVoxel = vioVoxelQueue.front();
		vioVoxelQueue.pop();

		for (const auto& Direction : Directions)
		{
			glm::ivec3 NextVoxel(CurrentVoxel + Direction);
			if (__isValidVoxel(NextVoxel))
			{
				int NextVoxelIndex = __transformIndex2Int(NextVoxel);
				if (m_VoxelStatusSet[NextVoxelIndex])
					continue;
				if (!TempVoxelStatusSet[NextVoxelIndex])
				{
					TempVoxelStatusSet[NextVoxelIndex] = true;
					vioVoxelQueue.push(NextVoxel);
				}
			}
		}
	}
	m_VoxelStatusSet.swap(TempVoxelStatusSet);
}

//*********************************************************************
//FUNCTION:
void Animation::CVoxelizer::__setupCountBuffer(GLuint& voCountBuffer, int vVoxelSetLength)
{
	_ASSERTE(vVoxelSetLength > 0);

	GLint ViewPort[4];
	glGetIntegerv(GL_VIEWPORT, ViewPort);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_MULTISAMPLE);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glGenBuffers(1, &voCountBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, voCountBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, vVoxelSetLength * sizeof(int), nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, voCountBuffer);
}

//*********************************************************************
//FUNCTION:
void Animation::CVoxelizer::__setupShader(graphics::CShader& vioShader, const glm::ivec3& vResolution)
{
	_ASSERTE(vResolution != glm::ivec3());

	const float Offset = 0.2f;
	const glm::vec3 BindResolution = { vResolution.x, vResolution.y, vResolution.z };
	const std::vector<SCameraInfo> CameraInfoSet = __generateCamerasPlacedAlongAxes();
	const glm::vec3 Range = { m_ModelAABB.MaxCoordinate.x - m_ModelAABB.MinCoordinate.x, m_ModelAABB.MaxCoordinate.y - m_ModelAABB.MinCoordinate.y , m_ModelAABB.MaxCoordinate.z - m_ModelAABB.MinCoordinate.z };

	vioShader.setVec3("boxMin", m_ModelAABB.MinCoordinate);
	vioShader.setFloat("step", m_VoxelSize);
	vioShader.setVec3("resolution", BindResolution);
	vioShader.setVec2("halfPixel[0]", glm::vec2(1.0f / vResolution.z, 1.0f / vResolution.y));
	vioShader.setVec2("halfPixel[1]", glm::vec2(1.0f / vResolution.x, 1.0f / vResolution.z));
	vioShader.setVec2("halfPixel[2]", glm::vec2(1.0f / vResolution.x, 1.0f / vResolution.y));

	glm::mat4 XViewMatrix = glm::lookAt(CameraInfoSet[0].Eye, CameraInfoSet[0].LookAt, CameraInfoSet[0].Up);
	glm::mat4 XProjectionMatrix = glm::ortho(-Range.z * 0.51f, +Range.z * 0.51f, -Range.y * 0.51f, +Range.y * 0.51f, m_VoxelSize, Range.x * 1.2f + Offset);
	vioShader.setMat4("viewProject[0]", XProjectionMatrix * XViewMatrix);
	vioShader.setMat4("viewProjectInverse[0]", glm::inverse(XProjectionMatrix * XViewMatrix));

	glm::mat4 YViewMatrix = glm::lookAt(CameraInfoSet[1].Eye, CameraInfoSet[1].LookAt, CameraInfoSet[1].Up);
	glm::mat4 YProjectionMatrix = glm::ortho(-Range.x * 0.51f, +Range.x * 0.51f, -Range.z * 0.51f, +Range.z * 0.51f, m_VoxelSize, Range.y * 1.2f + Offset);
	vioShader.setMat4("viewProject[1]", YProjectionMatrix * YViewMatrix);
	vioShader.setMat4("viewProjectInverse[1]", glm::inverse(YProjectionMatrix * YViewMatrix));

	glm::mat4 ZViewMatrix = glm::lookAt(CameraInfoSet[2].Eye, CameraInfoSet[2].LookAt, CameraInfoSet[2].Up);
	glm::mat4 ZProjectionMatrix = glm::ortho(-Range.x * 0.51f, +Range.x * 0.51f, -Range.y * 0.51f, +Range.y * 0.51f, m_VoxelSize, Range.z * 1.2f + Offset);
	vioShader.setMat4("viewProject[2]", ZProjectionMatrix * ZViewMatrix);
	vioShader.setMat4("viewProjectInverse[2]", glm::inverse(ZProjectionMatrix * ZViewMatrix));
}

//*********************************************************************
//FUNCTION:
void Animation::CVoxelizer::__tagSurfaceVoxels(const graphics::CShader& vShader, GLuint vCountBuffer, int vVoxelSetLength)
{
	_ASSERTE(vVoxelSetLength > 0);

	auto pWrite = reinterpret_cast<int*>(glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY));
	for (int x = 0; x < vVoxelSetLength; ++x)
		pWrite[x] = 0;
	if (!glUnmapBuffer(GL_SHADER_STORAGE_BUFFER))
		throw std::runtime_error("Unmap buffer error!");

	m_pModel->draw(vShader);

	auto pRead = reinterpret_cast<int*>(glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY));

	m_VoxelStatusSet.resize(vVoxelSetLength, false);
	if (pRead != nullptr)
	{
		for (int x = 0; x < vVoxelSetLength; ++x)
		{
			if (pRead[x] != 0)
			{
				m_VoxelStatusSet[x] = true;
			}
		}
		std::cout << "Surface: " << m_VoxelStatusSet.count() << std::endl;
	}
	else
		throw std::runtime_error("Read pointer is null!");

	glUnmapBuffer(vCountBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glDeleteBuffers(1, &vCountBuffer);
}

//*********************************************************************
//FUNCTION:
std::vector<Animation::SCameraInfo> Animation::CVoxelizer::__generateCamerasPlacedAlongAxes() const
{
	std::vector<Animation::SCameraInfo> CameraInfoSet;
	const glm::vec3 CentralCoordinate = (m_ModelAABB.MaxCoordinate + m_ModelAABB.MinCoordinate) / glm::vec3(2, 2, 2);

	Animation::SCameraInfo XCamera;
	XCamera.Eye = { m_ModelAABB.MaxCoordinate.x + m_VoxelSize, CentralCoordinate.y , CentralCoordinate.z };
	XCamera.LookAt = CentralCoordinate;
	XCamera.Up = glm::vec3(0, 1, 0);
	CameraInfoSet.push_back(XCamera);

	Animation::SCameraInfo YCamera;
	YCamera.Eye = { CentralCoordinate.x, m_ModelAABB.MaxCoordinate.y + m_VoxelSize, CentralCoordinate.z };
	YCamera.LookAt = CentralCoordinate;
	YCamera.Up = glm::vec3(0, 1.0, 0.001);
	CameraInfoSet.push_back(YCamera);

	Animation::SCameraInfo ZCamera;
	ZCamera.Eye = { CentralCoordinate.x, CentralCoordinate.y, m_ModelAABB.MaxCoordinate.z + m_VoxelSize };
	ZCamera.LookAt = CentralCoordinate;
	ZCamera.Up = glm::vec3(0, 1, 0);
	CameraInfoSet.push_back(ZCamera);

	return CameraInfoSet;
}

//*********************************************************************
//FUNCTION:
bool Animation::CVoxelizer::__isValidVoxel(glm::ivec3 vPoint) const
{
	return vPoint.x >= 0 && vPoint.y >= 0 && vPoint.z >= 0
		&& vPoint.x < m_VoxelResolution.x && vPoint.y < m_VoxelResolution.y && vPoint.z < m_VoxelResolution.z;
}

//*********************************************************************
//FUNCTION:
int Animation::CVoxelizer::__transformIndex2Int(const glm::ivec3& vIndex) const
{
	return vIndex.y * (m_VoxelResolution.x * m_VoxelResolution.z) + vIndex.z * m_VoxelResolution.x + vIndex.x;
}

void Animation::CVoxelizer::__generateVoxelsInfo(const std::vector<std::string>& vAllStrFromFile)
{
	std::vector<std::string> CurrentLineSplited;
	boost::split(CurrentLineSplited, vAllStrFromFile[TOTAL_VOXEL_SIZE_LINE], boost::is_any_of(","));

	auto x = boost::lexical_cast<int>(CurrentLineSplited[0]);
	auto y = boost::lexical_cast<int>(CurrentLineSplited[1]);
	auto z = boost::lexical_cast<int>(CurrentLineSplited[2]);
	m_VoxelResolution = glm::vec3(x, y, z);

	boost::split(CurrentLineSplited, vAllStrFromFile[SPACING_LINE], boost::is_any_of(","));
	m_VoxelSize = boost::lexical_cast<float>(CurrentLineSplited[0]);

	boost::split(CurrentLineSplited, vAllStrFromFile[START_POSITION_LINE], boost::is_any_of(","));
	auto r = boost::lexical_cast<float>(CurrentLineSplited[0]);
	auto g = boost::lexical_cast<float>(CurrentLineSplited[1]);
	auto b = boost::lexical_cast<float>(CurrentLineSplited[2]);
	m_ModelAABB.MinCoordinate = glm::vec3(r, g, b) - glm::vec3(m_VoxelSize / 2.0);
	m_ModelAABB.MaxCoordinate = m_ModelAABB.MinCoordinate + glm::vec3(
		m_VoxelSize * m_VoxelResolution.x,
		m_VoxelSize * m_VoxelResolution.y,
		m_VoxelSize * m_VoxelResolution.z
	);
}
