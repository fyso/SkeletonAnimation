#include <iostream>
#include <fstream>
#include <Windows.h>
#include <common/ConfigInterface.h>
#include "ApplicationConfig.h"
#include "AnimationConfig.h"
#include "GraphicsInterface.h"
#include <boost/shared_ptr.hpp>
#include <Shader.h>
#include <Sphere.h>
#include <Plane.h>
#include <Model.h>
#include "Voxelizer.h"
#include "Simulation.h"
#include "Skinner.h"

void installMemoryLeakDetector();
inline void parseConfig();
inline GLFWwindow* initWindow();
inline graphics::CCamera* initCamera(GLFWwindow* vWindow);
void processInput(GLFWwindow* vWindow);

void drawModel(Animation::CSkinner* vSkinner, const std::vector<Eigen::Vector3d>& vPosition, const boost::shared_ptr<graphics::CCamera> vCamera, const glm::mat4& vModel, const glm::mat4& vView, const glm::mat4& vProjection);
void drawParticles(Animation::CParticleSource* vParticleSource, Animation::CParticleLayer* vParticleLayer, const glm::mat4& vModel, const glm::mat4& vView, const glm::mat4& vProjection, const std::vector<Eigen::Vector3d>& vPositions);
void drawFloor(const glm::mat4& vModel, const glm::mat4& vView, const glm::mat4& vProjection);
void writeDataToTxt(const std::vector<Eigen::Vector3d>& vPositions);

bool isDrawParticle = true;
bool isStatic = false;
bool isCaptureFrameBuffer = true;
int  currentFrameIndex = 0;
bool isWrite = true;
int WatchCount = 0;
SYSTEMTIME OldUtcTime = { 0 };
SYSTEMTIME CurUtcTime = { 0 };
int OldTime = 0;
int CurTime = 0;
int TimeCost = 0;
std::vector<Eigen::Vector3d> Positions;
std::vector<Eigen::Vector3d> OldPositions;
GLuint CaptureTempBuffer;
int main()
{
	installMemoryLeakDetector();

	try
	{
		parseConfig();

		const int ScreenWidth = CApplicationConfig::getInstance()->getAttribute<int>(KEY_WORDS::SCREEN_WIDTH);
		const int ScreenHeight = CApplicationConfig::getInstance()->getAttribute<int>(KEY_WORDS::SCREEN_HEIGHT);
		const float NearPlane = CApplicationConfig::getInstance()->getAttribute<float>(KEY_WORDS::NEAR_PLANE);
		const float FarPlane = CApplicationConfig::getInstance()->getAttribute<float>(KEY_WORDS::FAR_PLANE);

		GLFWwindow *Window = initWindow();
		boost::shared_ptr<graphics::CCamera> Camera(initCamera(Window));

		//³õÊ¼»¯
		const std::string ModelPath = CApplicationConfig::getInstance()->getAttribute<std::string>(KEY_WORDS::MODEL_PATH);
		const std::string VoxelDataPath = CApplicationConfig::getInstance()->getAttribute<std::string>(KEY_WORDS::VOXEL_DATA_PATH);
		boost::shared_ptr<graphics::CModel> SimulationModel(new graphics::CModel(ModelPath));

		/*std::ofstream FileOut("./indices output/FatGuyJump_GPU_Indices.txt", std::fstream::out);
		const int MeshCount = SimulationModel->getMumOfMeshes();
		for (int i = 0; i < MeshCount; i++)
		{
			graphics::CMesh Mesh = SimulationModel->getMeshesAt(i);
			for (int k = 0; k < Mesh.getNumOfIndices(); k++)
			{
				unsigned int Index = Mesh.getIndexAt(k);
				FileOut << Index << std::endl;
			}
			FileOut << "END" << std::endl;
		}
		FileOut.close();

		FileOut.open("./indices output/FatGuyJump_GPU_Normal.txt", std::fstream::out);
		for (int i = 0; i < MeshCount; i++)
		{
			graphics::CMesh Mesh = SimulationModel->getMeshesAt(i);
			for (int k = 0; k < Mesh.getNumOfVertices(); k++)
			{
				graphics::SVertex Vertex = Mesh.getVertexAt(k);
				FileOut << Vertex.Normal.x << " " << Vertex.Normal.y << " " << Vertex.Normal.z << std::endl;
			}
			FileOut << "END" << std::endl;
		}
		FileOut.close();

		FileOut.open("./indices output/FatGuyJump_GPU_UV.txt", std::fstream::out);
		for (int i = 0; i < MeshCount; i++)
		{
			graphics::CMesh Mesh = SimulationModel->getMeshesAt(i);
			for (int k = 0; k < Mesh.getNumOfVertices(); k++)
			{
				graphics::SVertex Vertex = Mesh.getVertexAt(k);
				FileOut << Vertex.TexCoords.x << " " << Vertex.TexCoords.y << std::endl;
			}
			FileOut << "END" << std::endl;
		}
		FileOut.close();*/

		boost::shared_ptr<Animation::CVoxelizer> Voxelizer(new Animation::CVoxelizer);
		Voxelizer->voxelize(ModelPath);
		boost::shared_ptr<Animation::CParticleSource> ParticleSource(new Animation::CParticleSource);
		Voxelizer->sendVoxels(*ParticleSource);

		Voxelizer->saveVoxelData(VoxelDataPath);

		GetSystemTime(&OldUtcTime);
		ParticleSource->initialize();
		GetSystemTime(&CurUtcTime);
		OldTime = ((OldUtcTime.wHour * 60 + OldUtcTime.wMinute) * 60 + OldUtcTime.wSecond) * 1000 + OldUtcTime.wMilliseconds;
		CurTime = ((CurUtcTime.wHour * 60 + CurUtcTime.wMinute) * 60 + CurUtcTime.wSecond) * 1000 + CurUtcTime.wMilliseconds;
		TimeCost = CurTime - OldTime;
		std::cout << "Initialize Time Cost : " << TimeCost << std::endl;

		boost::shared_ptr<Animation::CSkeletonController> SkeletonController(new Animation::CSkeletonController);
		const std::string SkeletonFilePath = CApplicationConfig::getInstance()->getAttribute<std::string>(KEY_WORDS::SKELETON_FILE_PATH);
		SkeletonController->initSkeletonByFile(SkeletonFilePath);
		boost::shared_ptr<Animation::CParticleLayer> ParticleLayer(new Animation::CParticleLayer);
		ParticleLayer->initialize(ParticleSource.get(), SkeletonController.get());
		boost::shared_ptr<Animation::CSkinner> Skinner(new Animation::CSkinner(SimulationModel.get(), ParticleSource.get()));

		GetSystemTime(&OldUtcTime);
		Animation::CSimulation::getInstance()->initialize(ParticleSource.get(), ParticleLayer.get(), SkeletonController.get());
		GetSystemTime(&CurUtcTime);
		OldTime = ((OldUtcTime.wHour * 60 + OldUtcTime.wMinute) * 60 + OldUtcTime.wSecond) * 1000 + OldUtcTime.wMilliseconds;
		CurTime = ((CurUtcTime.wHour * 60 + CurUtcTime.wMinute) * 60 + CurUtcTime.wSecond) * 1000 + CurUtcTime.wMilliseconds;
		TimeCost = CurTime - OldTime;
		std::cout << "Color Graph Time Cost : " << TimeCost << std::endl;

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glGenBuffers(1, &CaptureTempBuffer);
		graphics::mainRunLoop(Window, [&]() {
			processInput(Window);

			glClearColor(0.93f, 0.93f, 0.93f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glm::mat4 Model = glm::mat4(1.0f);
			Model = glm::scale(Model, glm::vec3(0.1f));
			glm::mat4 View = Camera->getViewMatrix();
			glm::mat4 Projection = glm::perspective(Camera->getZoom(), (float)ScreenWidth / (float)ScreenHeight, NearPlane, FarPlane);
			OldPositions = Positions;

			OldUtcTime = { 0 };
			CurUtcTime = { 0 };

			GetSystemTime(&OldUtcTime);

			Positions = Animation::CSimulation::getInstance()->update();

			GetSystemTime(&CurUtcTime);
			OldTime = ((OldUtcTime.wHour * 60 + OldUtcTime.wMinute) * 60 + OldUtcTime.wSecond) * 1000 + OldUtcTime.wMilliseconds;
			CurTime = ((CurUtcTime.wHour * 60 + CurUtcTime.wMinute) * 60 + CurUtcTime.wSecond) * 1000 + CurUtcTime.wMilliseconds;
			TimeCost = CurTime - OldTime;
			
			std::ofstream FileOut("./FatGuyJump_GPU_8_2_4_1_LCF.txt", std::fstream::out | std::fstream::app);
			WatchCount++;
			if (WatchCount > 40 && WatchCount < 200)
			{
				for (int i = 0; i < Positions.size(); i++)
				{
					Eigen::Vector3d DeltaX = Positions[i] - OldPositions[i];
					FileOut << DeltaX.x() << " " << DeltaX.y() << " " << DeltaX.z() << " "  
						<< Positions[i].x() << " " << Positions[i].y() << " " << Positions[i].z() << " " << TimeCost << std::endl;
				}
				FileOut << "END" << std::endl;
			}
			FileOut.close();

			if (isDrawParticle)
			{
				drawParticles(ParticleSource.get(), ParticleLayer.get(), Model, View, Projection, Positions);
			}
			drawModel(Skinner.get(), Positions, Camera, Model, View, Projection);
			drawFloor(Model, View, Projection);

			if (isCaptureFrameBuffer && currentFrameIndex<300)
			{
				graphics::captureFrameBuffer(std::string("./output/") + std::to_string(currentFrameIndex) + std::string(".bmp"), ScreenWidth, ScreenHeight, &CaptureTempBuffer);
				currentFrameIndex++;
			}
		});

		graphics::closeWindow(Window);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		system("pause");
	}

	return 0;
}

//****************************************************************************************************
//FUNCTION: detect the memory leak in DEBUG mode
void installMemoryLeakDetector()
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	//_CRTDBG_LEAK_CHECK_DF: Perform automatic leak checking at program exit through a call to _CrtDumpMemoryLeaks and generate an error 
	//report if the application failed to free all the memory it allocated. OFF: Do not automatically perform leak checking at program exit.
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	//the following statement is used to trigger a breakpoint when memory leak happens
	//comment it out if there is no memory leak report;
	//_crtBreakAlloc = 68488;
#endif
}

void parseConfig()
{
	hiveConfig::EParseResult IsParsedApplicationConfig = hiveConfig::hiveParseConfig("ApplicationConfig.xml", hiveConfig::EConfigType::XML, CApplicationConfig::getInstance());
	if (IsParsedApplicationConfig != hiveConfig::EParseResult::SUCCEED)
	{
		throw std::runtime_error("Failed to parse application config file.");
	}

	IsParsedApplicationConfig = hiveConfig::hiveParseConfig("AnimationConfig.xml", hiveConfig::EConfigType::XML, Animation::CAnimationConfig::getInstance());
	if (IsParsedApplicationConfig != hiveConfig::EParseResult::SUCCEED)
	{
		throw std::runtime_error("Failed to parse animation config file.");
	}
}

GLFWwindow* initWindow()
{
	const int GLCoreMajor = CApplicationConfig::getInstance()->getAttribute<int>(KEY_WORDS::GL_CORE_MAJOR);
	const int GLCoreMinor = CApplicationConfig::getInstance()->getAttribute<int>(KEY_WORDS::GL_CORE_MINOR);
	const int ScreenWidth = CApplicationConfig::getInstance()->getAttribute<int>(KEY_WORDS::SCREEN_WIDTH);
	const int ScreenHeight = CApplicationConfig::getInstance()->getAttribute<int>(KEY_WORDS::SCREEN_HEIGHT);

	GLFWwindow *Window = nullptr;
	graphics::createWindow(Window, GLCoreMajor, GLCoreMinor, ScreenWidth, ScreenHeight);

	return Window;
}

graphics::CCamera* initCamera(GLFWwindow* vWindow)
{
	const std::string CameraPosStr = CApplicationConfig::getInstance()->getAttribute<std::string>(KEY_WORDS::INIT_CAMERA_POS);
	float Pos[3];
	hiveUtility::hiveString::hiveSplitLine2FloatArray(CameraPosStr, ",", 3, Pos);

	graphics::CCamera *Camera = new graphics::CCamera({ Pos[0], Pos[1], Pos[2] });
	graphics::createCamera(vWindow, Camera);

	return Camera;
}

void processInput(GLFWwindow* vWindow)
{
	// Enable/disable drawing status
	if (graphics::g_Keys[GLFW_KEY_F] && !graphics::g_KeysPressed[GLFW_KEY_F])
	{
		isStatic = !isStatic;
		graphics::g_KeysPressed[GLFW_KEY_F] = true;
	}
}

void drawModel(Animation::CSkinner* vSkinner, const std::vector<Eigen::Vector3d>& vPosition, const boost::shared_ptr<graphics::CCamera> vCamera, const glm::mat4& vModel, const glm::mat4& vView, const glm::mat4& vProjection)
{
	static bool IsInitialized = false;
	static boost::shared_ptr<graphics::CShader> ShaderForModel = nullptr;
	if (!IsInitialized)
	{
		const std::string ModelVertex = CApplicationConfig::getInstance()->getAttribute<std::string>(KEY_WORDS::SHADER_MODEL_VERTEX);
		const std::string ModelFragment = CApplicationConfig::getInstance()->getAttribute<std::string>(KEY_WORDS::SHADER_MODEL_FRAGMENT);
		ShaderForModel = boost::shared_ptr<graphics::CShader>(new graphics::CShader(ModelVertex.c_str(), ModelFragment.c_str()));

		IsInitialized = true;
	}

	if (ShaderForModel != nullptr && vSkinner != nullptr)
	{
		glCullFace(GL_FRONT);
		ShaderForModel->use();
		ShaderForModel->setMat4("model", vModel);
		ShaderForModel->setMat4("view", vView);
		ShaderForModel->setMat4("projection", vProjection);
		ShaderForModel->setVec3("ViewPos", vCamera->getPosition());
		ShaderForModel->setVec3("LightDir", glm::vec3(0, 0, -1.0f));
		vSkinner->updateAndDraw(*ShaderForModel, vPosition, true);
		glCullFace(GL_BACK);
	}
}

void drawParticles(Animation::CParticleSource* vParticleSource, Animation::CParticleLayer* vParticleLayer, const glm::mat4& vModel, const glm::mat4& vView, const glm::mat4& vProjection, const std::vector<Eigen::Vector3d>& vPositions)
{
	static bool IsInitialized = false;
	static int ParticleCount = 0;
	static GLuint OffsetInstanceID = 0;
	static boost::shared_ptr<graphics::CShader> ShaderForParticle = nullptr;
	static boost::shared_ptr<graphics::CSphere> Sphere = nullptr;
	if (!IsInitialized)
	{
		const std::string ParticleVertex = CApplicationConfig::getInstance()->getAttribute<std::string>(KEY_WORDS::SHADER_PARTICLE_VERTEX);
		const std::string ParticleFragment = CApplicationConfig::getInstance()->getAttribute<std::string>(KEY_WORDS::SHADER_PARTICLE_FRAGMENT);
		ShaderForParticle = boost::shared_ptr<graphics::CShader>(new graphics::CShader(ParticleVertex.c_str(), ParticleFragment.c_str()));

		const float Radius = vParticleSource->getParticleRadius();
		Sphere = boost::shared_ptr<graphics::CSphere>(new graphics::CSphere(Radius));

		ParticleCount = vParticleSource->getParticleCount();
		std::vector<glm::vec3> Translations(ParticleCount);
		std::vector<glm::vec3> Colors(ParticleCount);
		for (int i = 0; i < ParticleCount; i++)
		{
			Eigen::Vector3d Pos = vParticleSource->getParticlePos(i);
			Translations[i] = glm::vec3(Pos[0], Pos[1], Pos[2]);
			Eigen::Vector3f Color = vParticleLayer->getParticleColor(i);
			Colors[i] = glm::vec3(Color[0], Color[1], Color[2]);
		}
		OffsetInstanceID = Sphere->generateInstanceID();
		Sphere->setInstanceArray(OffsetInstanceID, 1, Translations, 3);
		GLuint ColorInstanceID = Sphere->generateInstanceID();
		Sphere->setInstanceArray(ColorInstanceID, 2, Colors, 3);

		IsInitialized = true;
	}

	if (ShaderForParticle != nullptr && Sphere != nullptr && ParticleCount > 0)
	{
		if (!isStatic)
		{
			std::vector<glm::vec3> Translations(ParticleCount);
			for (int i = 0; i < ParticleCount; i++)
			{
				Eigen::Vector3d WorldPos = vPositions[i];
				Translations[i] = glm::vec3(WorldPos[0], WorldPos[1], WorldPos[2]);
			}
			Sphere->setInstanceArray(OffsetInstanceID, 1, Translations, 3);
		}

		ShaderForParticle->use();
		ShaderForParticle->setMat4("model", vModel);
		ShaderForParticle->setMat4("view", vView);
		ShaderForParticle->setMat4("projection", vProjection);
		Sphere->drawInstance(*ShaderForParticle, ParticleCount);
	}
}

void drawFloor(const glm::mat4& vModel, const glm::mat4& vView, const glm::mat4& vProjection)
{
	static bool IsInitialized = false;
	static boost::shared_ptr<graphics::CShader> ShaderForFloor = nullptr;
	static boost::shared_ptr<graphics::CPlane> FloorPlane = nullptr;
	if (!IsInitialized)
	{
		const std::string FloorVertex = CApplicationConfig::getInstance()->getAttribute<std::string>(KEY_WORDS::SHADER_FLOOR_VERTEX);
		const std::string FloorFragment = CApplicationConfig::getInstance()->getAttribute<std::string>(KEY_WORDS::SHADER_FLOOR_FRAGMENT);
		ShaderForFloor = boost::shared_ptr<graphics::CShader>(new graphics::CShader(FloorVertex.c_str(), FloorFragment.c_str()));

		const float FloorWidth = CApplicationConfig::getInstance()->getAttribute<float>(KEY_WORDS::FLOOR_PLANE_WIDTH);
		const float FloorHeight = CApplicationConfig::getInstance()->getAttribute<float>(KEY_WORDS::FLOOR_PLANE_HEIGHT);
		FloorPlane = boost::shared_ptr<graphics::CPlane>(new graphics::CPlane(FloorWidth, FloorHeight));

		IsInitialized = true;
	}

	if (ShaderForFloor != nullptr && FloorPlane != nullptr)
	{
		ShaderForFloor->use();
		glm::mat4 Model = vModel;
		Model = glm::translate(Model, glm::vec3(0, -1.0f, 40.0f));

		ShaderForFloor->setMat4("model", Model);
		ShaderForFloor->setMat4("view", vView);
		ShaderForFloor->setMat4("projection", vProjection);
		FloorPlane->draw(*ShaderForFloor);
	}
}

void writeDataToTxt(const std::vector<Eigen::Vector3d>& vPositions)
{
	//std::string path = "Position" + std::to_string(currentFrameIndex) + ".txt";
	std::string path = "Position.txt";
	std::ofstream fout(path);
	if (fout)
	{
		fout << vPositions.size() << std::endl;
		for (int i = 0; i < vPositions.size(); ++i)
		{
			//fout << "x: " << vPositions[i].x << " " << "y: " << vPositions[i].y << " " << "z: " << vPositions[i].z << " " << "w: " << vPositions[i].w << " " << std::endl;
			fout << vPositions[i].x() << std::endl;
		}
		fout.close();
	}
}