#pragma once
#include <string>

namespace KEY_WORDS
{
	const std::string GL_CORE_MAJOR = "GL_CORE_MAJOR";
	const std::string GL_CORE_MINOR = "GL_CORE_MINOR";

	const std::string SCREEN_WIDTH = "SCREEN_WIDTH";
	const std::string SCREEN_HEIGHT = "SCREEN_HEIGHT";
	const std::string NEAR_PLANE = "NEAR_PLANE";
	const std::string FAR_PLANE = "FAR_PLANE";

	const std::string INIT_CAMERA_POS = "INIT_CAMERA_POS";

	const std::string MODEL_PATH = "MODEL_PATH";
	const std::string VOXEL_DATA_PATH = "VOXEL_DATA_PATH";
	const std::string SKELETON_FILE_PATH = "SKELETON_FILE_PATH";

	const std::string FLOOR_PLANE_WIDTH = "FLOOR_PLANE_WIDTH";
	const std::string FLOOR_PLANE_HEIGHT = "FLOOR_PLANE_HEIGHT";

	// Shaders
	const std::string SHADER_FLOOR_VERTEX = "SHADER_FLOOR_VERTEX";
	const std::string SHADER_FLOOR_FRAGMENT = "SHADER_FLOOR_FRAGMENT";
	const std::string SHADER_MODEL_VERTEX = "SHADER_MODEL_VERTEX";
	const std::string SHADER_MODEL_FRAGMENT = "SHADER_MODEL_FRAGMENT";
	const std::string SHADER_PARTICLE_VERTEX = "SHADER_PARTICLE_VERTEX";
	const std::string SHADER_PARTICLE_FRAGMENT = "SHADER_PARTICLE_FRAGMENT";
}