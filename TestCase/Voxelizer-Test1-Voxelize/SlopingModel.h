#pragma once
#include <glm/gtc/matrix_transform.hpp>

class CSlopingModel
{
public:
	explicit CSlopingModel(const glm::vec3 vRotation)
	{
		m_RotationMatX = m_RotationMatY = m_RotationMatZ = glm::mat4(1.0f);
		m_RotationMatX = glm::rotate(m_RotationMatX, glm::radians(-1 * vRotation.x), glm::vec3(1, 0, 0));
		m_RotationMatY = glm::rotate(m_RotationMatY, glm::radians(-1 * vRotation.y), glm::vec3(0, 1, 0));
		m_RotationMatZ = glm::rotate(m_RotationMatZ, glm::radians(-1 * vRotation.z), glm::vec3(0, 0, 1));
	}

	virtual ~CSlopingModel() = default;

protected:
	glm::mat4 m_RotationMatX;
	glm::mat4 m_RotationMatY;
	glm::mat4 m_RotationMatZ;
};