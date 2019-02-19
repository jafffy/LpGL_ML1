#include "FrameScalerSampleApp.h"
#include "ModelObj.h"
#include "Camera.h"
#include "Experiment.h"
#include "LpGLEngine.h"
#include "BoundingBox.h"

#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#include <vector>
#include <functional>
#include <random>
#include <algorithm>
#include <chrono>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/intersect.hpp"

#ifndef _WIN32
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include <ml_logging.h>
#endif

#include "Quad.h"

#ifdef MAXIMUM_ENERGY

#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>

#endif

class FrameScalerSampleAppImpl
{
public:
  bool is_LpGL_on = false;

	std::vector<ModelObj*> models;
	std::vector<ModelObj*> abnormalModels;
	std::vector<ModelObj*> normalModels;
	int targetFrameRate = 60;

	int randomLpGLStateIndex = 0;
/*
  eels_without_lpgl,
  eels_with_ds,
  eels_with_meshsimp,
  eels_with_culling,
  eels_with_full_lpgl,
*/
#ifdef LPGL_DS
	int currentLpGLState = eels_with_ds;
#elif LPGL_MESHSIMP
	int currentLpGLState = eels_with_meshsimp;
#elif LPGL_CULLING
	int currentLpGLState = eels_with_culling;
#elif LPGL_FULL
	int currentLpGLState = eels_with_full_lpgl;
#else
	int currentLpGLState = eels_without_lpgl;
#endif
	eExpermentLpGLState lpglStateSequence[eels_count + 1];

	eExpermentLpGLState lpglStateRandomSequence[eels_count + 1];
	eExperimentScenarioState scenarioState = eess_basic;

	float timer = 0.0f;

	void evaluateStateMachine();

	Quad quad;

	float position_weight = 0.5f;
	float dynamics = 1.0f;

#ifdef FIDELITY_SCENE
	int abnormal_random_index = 0;
#endif

#ifdef DYNAMIC_SCENE
	int numHit = 0;
	int numMissed = 0;
	int numTargets = 0;
#endif // DYNAMIC_SCENE

#ifdef MAXIMUM_ENERGY
	std::thread t;
#endif

	FrameScalerSampleAppImpl()
	{
		ML_LOG_TAG(Verbose, "BENCHMARK", "currentTest: %d", currentLpGLState);
	}

	void distributeObjects()
	{
#if defined(FIDELITY_SCENE)
		abnormal_random_index = random() % 3;

		switch (abnormal_random_index) {
		case 0:
			ML_LOG_TAG(Info, "FIDELITY", "Left eye");
			break;
		case 1:
			ML_LOG_TAG(Info, "FIDELITY", "Mouth");
			break;
		case 2:
			ML_LOG_TAG(Info, "FIDELITY", "Right eye");
			break;
		}

		models.clear();

		int normal_index = 0;
		int abnormal_index = 0;

		int n = normalModels.size() + abnormalModels.size();

		std::vector<int> rand_seq;

		rand_seq.push_back(random() % 8);

		auto get_unique_rand = [](std::vector<int>& existings) {
			int r = 0;
			bool distinct = false;

			while (!distinct) {
				distinct = true;

				r = random() % 8;

				for (int e : existings) {
					if (e == r) {
						distinct = false;
						break;
					}
				}
			}

			return r;
		};

		rand_seq.push_back(get_unique_rand(rand_seq));
		rand_seq.push_back(get_unique_rand(rand_seq));

		rand_seq.push_back((rand_seq[0] + 8) % 16);
		rand_seq.push_back((rand_seq[1] + 8) % 16);
		rand_seq.push_back((rand_seq[2] + 8) % 16);
		std::sort(rand_seq.begin(), rand_seq.end());

		auto contains = [](int target, std::vector<int>& v) {
			for (auto element : v) {
				if (target == element)
					return true;
			}

			return false;
		};

		for (int i = 0; i < n; ++i) {
			float t = (float)i / n;
			float c = 5.0f * cosf(t * 2 * M_PI);
			float s = 5.0f * sinf(t * 2 * M_PI);

			ModelObj* model = nullptr;

			if (contains(i, rand_seq)) {
				if (abnormal_index < abnormalModels.size()) {
					model = abnormalModels[abnormal_index];
					abnormal_index++;
				}
			}
			else {
				if (normal_index < normalModels.size()) {
					model = normalModels[normal_index];
					normal_index++;
				}
			}

			assert(model);

			model->SetRotation(glm::vec3(0, 0, -t * 2 * M_PI + M_PI * 0.5f + M_PI));
			model->SetPosition(glm::vec3(c, 0, s));

			models.push_back(model);
		}
#elif defined(DYNAMIC_SCENE)
		for (int i = 0; i < models.size(); ++i) {
			auto* model = models[i];

			model->SetIsPhysicalObject(true);
			model->Reset(0.5f, 1.0f);
			numTargets++;
		}
#else
		int n = models.size();

		for (int i = 0; i < n; ++i) {
			float t = (float)i / n;
			float c = 5.0f * cosf(t * 2 * M_PI * 0.01f);
			float s = 5.0f * sinf(t * 2 * M_PI * 0.01f);

			ModelObj* model = models[i];

			model->SetRotation(glm::vec3(0, 0, -t * 2 * M_PI + M_PI * 0.5f + M_PI));
			model->SetPosition(glm::vec3(c, 0, s));
		}
#endif
	}

	bool generateModels()
	{
#ifdef FIDELITY_SCENE
#ifdef EQUAL_ENERGY
		static const char* abnormal_paths[] = {
			ABNORMAL_MODEL_FILEPATH_HALF,
			ABNORMAL2_MODEL_FILEPATH_HALF,
			ABNORMAL3_MODEL_FILEPATH_HALF
		};

		static const char* abnormal_paths_res1[] = {
			ABNORMAL_MODEL_FILEPATH_HALF,
			ABNORMAL2_MODEL_FILEPATH_HALF,
			ABNORMAL3_MODEL_FILEPATH_HALF
		};

		static const char* abnormal_paths_res2[] = {
			ABNORMAL_MODEL_FILEPATH_HALF,
			ABNORMAL2_MODEL_FILEPATH_HALF,
			ABNORMAL3_MODEL_FILEPATH_HALF
		};

#else
		static const char* abnormal_paths[] = {
			ABNORMAL_MODEL_FILEPATH,
			ABNORMAL2_MODEL_FILEPATH,
			ABNORMAL3_MODEL_FILEPATH
		};

		static const char* abnormal_paths_res1[] = {
			ABNORMAL_MODEL_FILEPATH_REDUCED_1,
			ABNORMAL2_MODEL_FILEPATH_REDUCED_1,
			ABNORMAL3_MODEL_FILEPATH_REDUCED_1
		};

		static const char* abnormal_paths_res2[] = {
			ABNORMAL_MODEL_FILEPATH_REDUCED_2,
			ABNORMAL2_MODEL_FILEPATH_REDUCED_2,
			ABNORMAL3_MODEL_FILEPATH_REDUCED_2
		};
#endif

		for (int i = 0; i < 6; ++i) {
			const char* model_path = abnormal_paths[i % 3];
			const char* model_path_res1 = abnormal_paths_res1[i % 3];
			const char* model_path_res2 = abnormal_paths_res2[i % 3];

			auto model = new ModelObj();
			model->Load(model_path,
				model_path_res1,
				model_path_res2, TARGET_MODEL_BASEPATH);
			model->SetShaders(VS_FILE_PATH, FS_FILE_PATH);

			model->SetScale(glm::vec3(0.25f));
			model->SetVisible(true);
			model->SetIsPhysicalObject(false);
			model->SetAbnormal(i);

			if (!model->Create())
				return false;

			abnormalModels.push_back(model);
		}
#endif

#ifdef FIDELITY_SCENE
		int n = NUM_OBJECTS - abnormalModels.size();
#else
		int n = NUM_OBJECTS;
#endif

		for (int i = 0; i < n; ++i) {
			auto model = new ModelObj();
#ifdef EQUAL_ENERGY
			model->Load(TARGET_MODEL_FILEPATH_HALF,
				TARGET_MODEL_FILEPATH_HALF,
				TARGET_MODEL_FILEPATH_HALF, TARGET_MODEL_BASEPATH);
#else
			model->Load(TARGET_MODEL_FILEPATH,
				TARGET_MODEL_FILEPATH_REDUCED_1,
				TARGET_MODEL_FILEPATH_REDUCED_2, TARGET_MODEL_BASEPATH);
#endif
			model->SetShaders(VS_FILE_PATH, FS_FILE_PATH);

			model->SetScale(glm::vec3(1.0f));
			model->SetVisible(true);
			if (!model->Create())
				return false;

			assert(model);
#ifdef FIDELITY_SCENE
			normalModels.push_back(model);
#else
			models.push_back(model);
#endif
		}

		distributeObjects();

		return true;
	}
};

#ifdef MAXIMUM_ENERGY
#define BUFSIZE 8196
static char buf[BUFSIZE];
static const char* server_name = "210.107.198.216";
static const int server_port = 7080;
#endif

FrameScalerSampleApp::FrameScalerSampleApp()
{
	impl = new FrameScalerSampleAppImpl();
}

FrameScalerSampleApp::~FrameScalerSampleApp()
{
	if (impl) {
		delete impl;
		impl = nullptr;
	}
}

int FrameScalerSampleApp::Start()
{
	return 0;
}

void FrameScalerSampleApp::Cleanup()
{
}

void FrameScalerSampleAppImpl::evaluateStateMachine()
{
	const float session_period = 5.0f;

	const float k_high_dynamics = 2.0f;
	const float k_low_dynamics = 0.5f;
	const float k_mid_dynamics = 1.0f;

	const float k_left_position = 0.90f;
	const float k_right_position = 0.10f;
	const float k_mid_position = 0.50f;

	if (scenarioState == eess_basic) {
		if (timer > session_period) {
			ML_LOG_TAG(Verbose, "BENCHMARK", "eess_high_dynamics");
			dynamics = k_high_dynamics;
			scenarioState = eess_high_dynamics;
			timer = 0.0f;

			for (auto* model : models)
				model->Reset(k_mid_position, k_high_dynamics);
		}
	}
	else if (scenarioState == eess_high_dynamics) {
		if (timer > session_period) {
			ML_LOG(Verbose, "eess_low_dynamics");
			dynamics = k_low_dynamics;
			scenarioState = eess_low_dynamics;
			timer = 0.0f;

			for (auto* model : models)
				model->Reset(k_mid_position, k_low_dynamics);
		}
	}
	else if (scenarioState == eess_low_dynamics) {
		if (timer > session_period) {
			ML_LOG(Verbose, "eess_left_most");
			dynamics = k_mid_dynamics;
			position_weight = k_left_position;
			scenarioState = eess_left_most;
			timer = 0.0f;

			for (auto* model : models)
				model->Reset(k_left_position, k_mid_dynamics);
		}
	}
	else if (scenarioState == eess_left_most) {
		if (timer > session_period) {
			ML_LOG(Verbose, "eess_right_most");
			dynamics = k_mid_dynamics;
			position_weight = k_right_position;
			scenarioState = eess_right_most;
			timer = 0.0f;

			for (auto* model : models)
				model->Reset(k_right_position, k_mid_dynamics);
		}
	}
	else if (scenarioState == eess_right_most) {
		if (timer > session_period) {
			ML_LOG(Verbose, "eess_basic");
			dynamics = k_mid_dynamics;
			position_weight = k_mid_position;
			scenarioState = eess_basic;
			timer = 0.0f;

			for (auto* model : models)
				model->Reset(k_mid_position, k_mid_dynamics);
		}
	}
}

void FrameScalerSampleApp::Update(float dt)
{
	float timer = impl->timer;

	impl->evaluateStateMachine();

	impl->timer += dt;

	for (auto* model : impl->models) {
		if (model->IsVisible()) {
			const auto& p = model->GetPosition();

			if (p.y < -1) {
				model->Reset(0.5f, 1.0f);
			}

			model->Update(dt);
		}
	}
}

void FrameScalerSampleApp::OnRender(int cameraIndex, float dt)
{
	static bool isFirstRender = true;

	if (isFirstRender) {
		ML_LOG_TAG(Info, LATENCY, "First Render");
		isFirstRender = false;
	}

	Update(dt);

  if (cameraIndex == 0 // LpGL for one eye.
      && impl->is_LpGL_on) {
    ML_LOG(Info, "Run LpGL");

    auto& engine = LpGLEngine::instance();
    auto state = eels_with_full_lpgl;

		impl->targetFrameRate = engine.Update(state, impl->models,
                                          impl->targetFrameRate, dt);
  }

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (auto* model : impl->models)
		model->Render();
}

int FrameScalerSampleApp::GetTargetFrameRate()
{
		return impl->targetFrameRate;
}

void FrameScalerSampleApp::SetTargetFrameRate(int targetFrameRate)
{
	impl->targetFrameRate = targetFrameRate;
}

bool FrameScalerSampleApp::InitContents()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	impl->generateModels();

	impl->quad.InitContents();

	return true;
}

void FrameScalerSampleApp::DestroyContents()
{
	impl->quad.DestroyContents();

	for (auto* model : impl->models) {
		model->Destroy();

		if (model) {
			delete model;
			model = nullptr;
		}
	}

	impl->models.clear();
}

void FrameScalerSampleApp::OnPressed()
{
  bool old_, new_;

  old_ = impl->is_LpGL_on;
  new_ = !old_;

  impl->is_LpGL_on = new_;

  ML_LOG(Info, "Toggle LpGL: %d -> %d", old_, new_);
}

