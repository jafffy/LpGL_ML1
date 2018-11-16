#ifndef CAMERA_H_
#define CAMERA_H_

#include "glm/glm.hpp"

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
	float ratio;
};

#endif // CAMERA_H_