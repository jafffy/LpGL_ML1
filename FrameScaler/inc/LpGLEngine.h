#ifndef LPGLENGINE_H_
#define LPGLENGINE_H_

#include "ModelObj.h"

#include <vector>

class LpGLEngineImpl;

enum eExpermentLpGLState
{
	eels_without_lpgl,
	eels_with_ds,
	eels_with_meshsimp,
	eels_with_culling,
	eels_with_full_lpgl,
	eels_count
};

enum eExperimentScenarioState
{
	eess_basic,
	eess_high_dynamics,
	eess_low_dynamics,
	eess_left_most,
	eess_right_most
};

class LpGLEngine
{
public:
	LpGLEngine();
	~LpGLEngine();

	int Update(int currentState, std::vector<ModelObj*>& models, int currentFPS, float dt);

	void SetLODSensitivity(float angle);

	static LpGLEngine& instance();

private:
	LpGLEngineImpl* impl = nullptr;
};

#endif // LPGLENGINE_H_