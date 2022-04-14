#pragma once
#include <string>

namespace Animation
{
	namespace KEY_WORDS
	{
		const std::string VERTEX_SHADER = "VERTEX_SHADER";
		const std::string GEOMETRY_SHADER = "GEOMETRY_SHADER";
		const std::string FRAGMENT_SHADER = "FRAGMENT_SHADER";

		const std::string VOXEL_SIZE = "VOXEL_SIZE";
		const std::string AABB_PADDING_RATIO = "AABB_PADDING_RATIO";

		const std::string MUSCLE_FAT_RATIO = "MUSCLE_FAT_RATIO";
		const std::string SKINNING_ANGLE_TOLERANCE = "SKINNING_ANGLE_TOLERANCE";

		const std::string STIFFNESS_VALUE = "STIFFNESS_VALUE";
		const std::string SMOOTH_PARAMETER = "SMOOTH_PARAMETER";

		const std::string GRAVITY = "GRAVITY";
		const std::string ITERATION_COUNT = "ITERATION_COUNT";
		const std::string DAMP_FACTOR = "DAMP_FACTOR";
		const std::string TIME_STEP_SIZE = "TIME_STEP_SIZE";
		const std::string SIMULATION_COUNT_PER_FRAME = "SIMULATION_COUNT_PER_FRAME";
	}
}