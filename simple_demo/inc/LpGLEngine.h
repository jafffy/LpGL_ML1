#ifndef LPGLENGINE_H_
#define LPGLENGINE_H_

#include "ModelObj.h"

#include <vector>

class LpGLEngineImpl;

class LpGLEngine
{
public:
	LpGLEngine();
	~LpGLEngine();

  int Update(std::vector<ModelObj *> &models, int currentFPS, float dt);

	static LpGLEngine& instance();

  bool IsOn = false;

private:
	LpGLEngineImpl* impl = nullptr;
};

#endif // LPGLENGINE_H_