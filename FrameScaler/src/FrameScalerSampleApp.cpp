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

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/intersect.hpp"

#ifndef _WIN32
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include <ml_logging.h>
#endif

#include "Quad.h"

class FrameScalerSampleAppImpl
{
public:
	std::vector<ModelObj*> models;
	std::vector<ModelObj*> abnormalModels;
	int targetFrameRate = 60;

	int randomLpgLStateIndex = 0;
/*
  eels_without_lpgl,
  eels_with_ds,
  eels_with_meshsimp,
  eels_with_culling,
  eels_with_full_lpgl,
*/
	int currentLpGLState = eels_without_lpgl;
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
#endif // DYNAMIC_SCENE

	FrameScalerSampleAppImpl()
	{
		ML_LOG_TAG(Verbose, "BENCHMARK", "currentTest: %d", currentLpGLState);
	}

	void distributeObjects()
	{
#if defined(FIDELITY_SCENE)
		abnormal_random_index = random() % 3;
		ML_LOG_TAG(Info, "FIDELITY", "%d", abnormal_random_index);

		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(models.begin(), models.end(), g);

		int n = models.size();

		for (int i = 0; i < n; ++i) {
			float t = (float)(i - 3) / n;
			float c = 5.0f * cosf(t * 2 * M_PI);
			float s = 5.0f * sinf(t * 2 * M_PI);

			ModelObj* model = models[i];

			model->SetRotation(glm::vec3(0, 0, -t * 2 * M_PI + M_PI * 0.5f + M_PI));
			model->SetPosition(glm::vec3(c, 0, s));
		}
#elif defined(DYNAMIC_SCENE)
		for (int i = 0; i < models.size(); ++i) {
			auto* model = models[i];

			model->SetIsPhysicalObject(true);
			model->Reset(0.5f, 1.0f);
		}
#else
		int n = models.size();

		for (int i = 0; i < n; ++i) {
			float t = (float)i / n;
			float c = 5.0f * cosf(t * 2 * M_PI);
			float s = 5.0f * sinf(t * 2 * M_PI);

			ModelObj* model = models[i];

			model->SetRotation(glm::vec3(0, 0, -t * 2 * M_PI + M_PI * 0.5f + M_PI));
			model->SetPosition(glm::vec3(c, 0, s));
		}
#endif
	}

	bool generateModels()
	{
#ifdef FIDELITY_SCENE
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

		for (int i = 0; i < 3; ++i) {
			const char* model_path = abnormal_paths[i];
			const char* model_path_res1 = abnormal_paths_res1[i];
			const char* model_path_res2 = abnormal_paths_res2[i];

			auto model = new ModelObj();
			model->Load(model_path,
				model_path_res1,
				model_path_res2, TARGET_MODEL_BASEPATH);
			model->SetShaders(VS_FILE_PATH, FS_FILE_PATH);

			model->SetScale(glm::vec3(0.25f));
			model->SetVisible(true);
			model->SetIsPhysicalObject(false);

			if (!model->Create())
				return false;

			models.push_back(model);
			abnormalModels.push_back(model);
		}
#endif

#ifdef FIDELITY_SCENE
		int n = NUM_OBJECTS - 3;
#else
		int n = NUM_OBJECTS;
#endif

		for (int i = 0; i < n; ++i) {
			auto model = new ModelObj();
			model->Load(TARGET_MODEL_FILEPATH,
				TARGET_MODEL_FILEPATH_REDUCED_1,
				TARGET_MODEL_FILEPATH_REDUCED_2, TARGET_MODEL_BASEPATH);
			model->SetShaders(VS_FILE_PATH, FS_FILE_PATH);

			model->SetScale(glm::vec3(0.25f));
			model->SetVisible(true);
			if (!model->Create())
				return false;

			models.push_back(model);
		}

		distributeObjects();

		return true;
	}
};

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

	auto currentState = impl->lpglStateSequence[impl->currentLpGLState];

	Update(dt);

	if (cameraIndex == 0) {
		int recommended_fps = LpGLEngine::instance().Update(impl->currentLpGLState, impl->models, GetTargetFrameRate(), dt);
		SetTargetFrameRate(recommended_fps);
	}

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (auto* model : impl->models)
		model->Render();

	impl->quad.Draw();
}

int FrameScalerSampleApp::GetTargetFrameRate()
{
	auto currentState = impl->lpglStateSequence[impl->currentLpGLState];

	if (currentState == eels_with_full_lpgl
		|| currentState == eels_with_ds)
		return impl->targetFrameRate;
	else
		return 60;
}

void FrameScalerSampleApp::SetTargetFrameRate(int targetFrameRate)
{
	impl->targetFrameRate = targetFrameRate;
}

bool FrameScalerSampleApp::InitContents()
{
	srandom(201120848);

	impl->lpglStateSequence[0] = eels_without_lpgl;
	impl->lpglStateSequence[1] = eels_with_ds;
	impl->lpglStateSequence[2] = eels_with_meshsimp;
	impl->lpglStateSequence[3] = eels_with_culling;
	impl->lpglStateSequence[4] = eels_with_full_lpgl;

	int currentLpGLstateSeq = 0;

	while (currentLpGLstateSeq < eels_count) {
		auto state = (eExpermentLpGLState)(random() % eels_count);

		for (int j = 0; j < currentLpGLstateSeq; ++j) {
			if (impl->lpglStateRandomSequence[j] == state) {
				continue;
			}
		}

		impl->lpglStateRandomSequence[currentLpGLstateSeq++] = state;
	}

#ifdef TRANSIT_LPGL_STATE
	impl->currentLpGLState = impl->lpglStateRandomSequence[impl->randomLpgLStateIndex++];
	ML_LOG_TAG(Info, "STATE", "%d\n", impl->currentLpGLState);
#endif

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
	ModelObj* closestModel = nullptr;
	float maxDistance = -FLT_MAX;

	for (auto* model : impl->models) {
		std::vector<glm::vec3> boundingVertices = model->GetBoundingBox();
		auto boundingBox2D = new BoundingBox2D();

		for (const auto& v : boundingVertices) {
			glm::vec4 result = Camera::Instance().P_for_LpGL * (Camera::Instance().V_for_LpGL * glm::vec4(v, 1.0f));

			boundingBox2D->AddPoint(result.x, result.y);

			if (!(boundingBox2D->Min.y > 0.01 || boundingBox2D->Max.y < -0.01
				|| boundingBox2D->Min.x > 0.01 || boundingBox2D->Max.x < -0.01)) {

				if (maxDistance < result.z) {
					closestModel = model;
					maxDistance = result.z;
				}
			}
		}
	}

	if (!closestModel) {
#ifdef DYNAMIC_SCENE
		impl->numMissed++;
		ML_LOG_TAG(Info, HIT_ACCURACY, "hit: %d, miss: %d, acc: %f", impl->numHit, impl->numMissed, (float)impl->numHit / (impl->numHit + impl->numMissed));
#endif // DYNAMIC_SCENE
		return;
	}

#if defined(FIDELITY_SCENE)
	if (closestModel == impl->abnormalModels[impl->abnormal_random_index]) {
    switch (impl->abnormal_random_index) {
    case 0:
      ML_LOG(Info, "FIDELITY", "Left eye");
      break;
    case 1:
      ML_LOG(Info, "FIDELITY", "Mouth");
      break;
    case 2:
      ML_LOG(Info, "FIDELITY", "Right eye");
      break;
    }

		ML_LOG_TAG(Info, FIDELITY_LATENCY, "Find!");

		impl->distributeObjects();

#ifdef TRANSIT_LPGL_STATE
		impl->currentLpGLState = impl->lpglStateRandomSequence[impl->randomLpgLStateIndex++];
		ML_LOG_TAG(Info, "STATE", "%d\n", impl->currentLpGLState);
#endif

		ML_LOG_TAG(Info, FIDELITY_LATENCY, "Start to finding");
	}
#elif defined(DYNAMIC_SCENE)
	impl->numHit++;
	ML_LOG_TAG(Info, HIT_ACCURACY, "hit: %d, miss: %d, acc: %f", impl->numHit, impl->numMissed, (float)impl->numHit / (impl->numHit + impl->numMissed));
#endif

#ifdef DYNAMIC_SCENE
	closestModel->Reset(0.5f, 1.0f);
#endif
	}
