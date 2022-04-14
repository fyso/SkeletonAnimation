#pragma once
#include "ApplicationCommon.h"
#include <common/HiveConfig.h>
#include <common/Singleton.h>

class CApplicationConfig : public hiveConfig::CHiveConfig, public hiveDesignPattern::CSingleton<CApplicationConfig>
{
public:
	virtual ~CApplicationConfig() = default;

private:
	CApplicationConfig() { __defineAttributesV(); }

	void __defineAttributesV()
	{
		_defineAttribute(KEY_WORDS::GL_CORE_MAJOR, hiveConfig::EConfigDataType::ATTRIBUTE_INT);
		_defineAttribute(KEY_WORDS::GL_CORE_MINOR, hiveConfig::EConfigDataType::ATTRIBUTE_INT);

		_defineAttribute(KEY_WORDS::SCREEN_WIDTH, hiveConfig::EConfigDataType::ATTRIBUTE_INT);
		_defineAttribute(KEY_WORDS::SCREEN_HEIGHT, hiveConfig::EConfigDataType::ATTRIBUTE_INT);
		_defineAttribute(KEY_WORDS::NEAR_PLANE, hiveConfig::EConfigDataType::ATTRIBUTE_FLOAT);
		_defineAttribute(KEY_WORDS::FAR_PLANE, hiveConfig::EConfigDataType::ATTRIBUTE_FLOAT);

		_defineAttribute(KEY_WORDS::INIT_CAMERA_POS, hiveConfig::EConfigDataType::ATTRIBUTE_STRING);

		_defineAttribute(KEY_WORDS::MODEL_PATH, hiveConfig::EConfigDataType::ATTRIBUTE_STRING);
		_defineAttribute(KEY_WORDS::VOXEL_DATA_PATH, hiveConfig::EConfigDataType::ATTRIBUTE_STRING);
		_defineAttribute(KEY_WORDS::SKELETON_FILE_PATH, hiveConfig::EConfigDataType::ATTRIBUTE_STRING);

		_defineAttribute(KEY_WORDS::FLOOR_PLANE_WIDTH, hiveConfig::EConfigDataType::ATTRIBUTE_FLOAT);
		_defineAttribute(KEY_WORDS::FLOOR_PLANE_HEIGHT, hiveConfig::EConfigDataType::ATTRIBUTE_FLOAT);

		// Shaders
		_defineAttribute(KEY_WORDS::SHADER_FLOOR_VERTEX, hiveConfig::EConfigDataType::ATTRIBUTE_STRING);
		_defineAttribute(KEY_WORDS::SHADER_FLOOR_FRAGMENT, hiveConfig::EConfigDataType::ATTRIBUTE_STRING);
		_defineAttribute(KEY_WORDS::SHADER_MODEL_VERTEX, hiveConfig::EConfigDataType::ATTRIBUTE_STRING);
		_defineAttribute(KEY_WORDS::SHADER_MODEL_FRAGMENT, hiveConfig::EConfigDataType::ATTRIBUTE_STRING);
		_defineAttribute(KEY_WORDS::SHADER_PARTICLE_VERTEX, hiveConfig::EConfigDataType::ATTRIBUTE_STRING);
		_defineAttribute(KEY_WORDS::SHADER_PARTICLE_FRAGMENT, hiveConfig::EConfigDataType::ATTRIBUTE_STRING);
	}

	friend class hiveDesignPattern::CSingleton<CApplicationConfig>;
};