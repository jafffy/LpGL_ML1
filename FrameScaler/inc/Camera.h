#ifndef CAMERA_H_
#define CAMERA_H_

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

class Camera
{
public:
	static Camera& Instance()
	{
		static Camera instance;
		return instance;
	}

	glm::mat4 V;
	glm::mat4 P;
	glm::mat4 P_for_LpGL;

	float ratio;

	glm::vec3 position;
	glm::quat rotation;
};

#endif // CAMERA_H_