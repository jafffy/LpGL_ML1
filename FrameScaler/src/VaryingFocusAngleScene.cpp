#include "VaryingFocusAngleScene.h"
#include "Quad.h"
#include "ModelObj.h"
#include "LpGLEngine.h"
#include "Experiment.h"

#include <ml_logging.h>

#include <vector>

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

class VaryingFocusAngleSceneImpl
{
public:
	Quad quad;
	int targetFrameRate = 60;
	std::vector<ModelObj*> models;
	float simplificationAngularFactor = 0.0f;
};

VaryingFocusAngleScene::VaryingFocusAngleScene()
{
	impl = new VaryingFocusAngleSceneImpl();
}

VaryingFocusAngleScene::~VaryingFocusAngleScene()
{
	if (impl) {
		delete impl;
		impl = nullptr;
	}
}

bool VaryingFocusAngleScene::InitContents()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	int n = 16;

	for (int i = 0; i < n; ++i) {
		float t = (float)i / n;
		float c = 5.0f * cosf(t * 2 * M_PI * 30.0f / 180.0f - M_PI * 0.5f);
		float s = 5.0f * sinf(t * 2 * M_PI * 30.0f / 180.0f - M_PI * 0.5f);

		auto model = new ModelObj();
		model->Load(TARGET_MODEL_FILEPATH,
			TARGET_MODEL_FILEPATH_REDUCED_1,
			TARGET_MODEL_FILEPATH_REDUCED_2,
			TARGET_MODEL_BASEPATH);

		if (i == 8)
			model->SetShaders(VS_FILE_PATH, GREEN_FS_FILE_PATH);
		else
			model->SetShaders(VS_FILE_PATH, FS_FILE_PATH);

		model->SetPosition(glm::vec3(c, 0, s));
		model->SetRotation(glm::vec3(0, 0, -t * 2 * M_PI + M_PI * 0.5f + M_PI));
		model->SetScale(glm::vec3(0.15f));
		model->SetVisible(true);

		if (!model->Create())
			return false;

		impl->models.push_back(model);
	}


	impl->quad.InitContents();

	return true;
}

void VaryingFocusAngleScene::DestroyContents()
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

int VaryingFocusAngleScene::GetTargetFrameRate()
{
	return impl->targetFrameRate;
}

void VaryingFocusAngleScene::OnRender(int cameraIndex, float dt)
{
	impl->simplificationAngularFactor += M_PI / 180.0f * dt * 0.5f;

	static bool isFirstRender = true;

	if (isFirstRender) {
		ML_LOG_TAG(Info, LATENCY, "First Render");
		isFirstRender = false;
	}

	for (auto* model : impl->models) {
		model->Update(dt);
	}

	if (cameraIndex == 0) {
		int recommended_fps = LpGLEngine::instance().Update(eels_with_full_lpgl, impl->models, impl->targetFrameRate, dt);
		impl->targetFrameRate = recommended_fps;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (auto* model : impl->models) {
		model->Render();
	}

	impl->quad.Draw();
}

void VaryingFocusAngleScene::OnPressed()
{
	ML_LOG_TAG(Info, "SIMPLIFY", "Current SIMPLIFY FOV: %f", (LOD_LV2 - impl->simplificationAngularFactor) / M_PI * 180);
}