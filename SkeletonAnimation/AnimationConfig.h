#pragma once
#include "AnimationCommon.h"
#include <common/HiveConfig.h>
#include <common/Singleton.h>

namespace Animation
{
	class CAnimationConfig : public hiveConfig::CHiveConfig, public hiveDesignPattern::CSingleton<CAnimationConfig>
	{
	public:
		virtual ~CAnimationConfig() = default;

	private:
		CAnimationConfig() { __defineAttributesV(); }

		void __defineAttributesV()
		{
			_defineAttribute(KEY_WORDS::VERTEX_SHADER, hiveConfig::EConfigDataType::ATTRIBUTE_STRING);
			_defineAttribute(KEY_WORDS::GEOMETRY_SHADER, hiveConfig::EConfigDataType::ATTRIBUTE_STRING);
			_defineAttribute(KEY_WORDS::FRAGMENT_SHADER, hiveConfig::EConfigDataType::ATTRIBUTE_STRING);

			_defineAttribute(KEY_WORDS::VOXEL_SIZE, hiveConfig::EConfigDataType::ATTRIBUTE_FLOAT);
			_defineAttribute(KEY_WORDS::AABB_PADDING_RATIO, hiveConfig::EConfigDataType::ATTRIBUTE_FLOAT);

			_defineAttribute(KEY_WORDS::MUSCLE_FAT_RATIO, hiveConfig::EConfigDataType::ATTRIBUTE_DOUBLE);
			_defineAttribute(KEY_WORDS::SKINNING_ANGLE_TOLERANCE, hiveConfig::EConfigDataType::ATTRIBUTE_DOUBLE);

			_defineAttribute(KEY_WORDS::STIFFNESS_VALUE, hiveConfig::EConfigDataType::ATTRIBUTE_STRING);
			_defineAttribute(KEY_WORDS::SMOOTH_PARAMETER, hiveConfig::EConfigDataType::ATTRIBUTE_DOUBLE);

			_defineAttribute(KEY_WORDS::GRAVITY, hiveConfig::EConfigDataType::ATTRIBUTE_STRING);
			_defineAttribute(KEY_WORDS::ITERATION_COUNT, hiveConfig::EConfigDataType::ATTRIBUTE_INT);
			_defineAttribute(KEY_WORDS::DAMP_FACTOR, hiveConfig::EConfigDataType::ATTRIBUTE_DOUBLE);
			_defineAttribute(KEY_WORDS::TIME_STEP_SIZE, hiveConfig::EConfigDataType::ATTRIBUTE_DOUBLE);
			_defineAttribute(KEY_WORDS::SIMULATION_COUNT_PER_FRAME, hiveConfig::EConfigDataType::ATTRIBUTE_INT);
		}

		friend class hiveDesignPattern::CSingleton<CAnimationConfig>;
	};
}