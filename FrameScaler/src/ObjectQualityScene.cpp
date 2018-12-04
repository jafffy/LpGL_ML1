#include "ObjectQuqlityScene.h"
#include "Quad.h"
#include "ModelObj.h"
#include "LpGLEngine.h"
#include "Experiment.h"

#include <ml_logging.h>

#include <vector>
#include <cstdlib>

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

char* MODEL_PATH_OF(const char* g) {
	char buf[4096] = { 0, };

	sprintf(buf, "%s%s.obj", TARGET_MODEL_BASEPATH, g);

	return strdup(buf);
}

char* REDUCED_MODEL_PATH_OF(const char* g, int level) {
	char buf[4096] = { 0, };

	sprintf(buf, "%s%s_%d.obj", TARGET_MODEL_BASEPATH, g, level);
	
	return strdup(buf);
}

char* REDUCED_MODEL_PATH_OF(const char* g, float reduction_rate) {
	char buf[4096] = { 0, };

	sprintf(buf, "%s%s_%.1f.obj", TARGET_MODEL_BASEPATH, g, reduction_rate);

	return strdup(buf);
}

class ObjectQualitySceneImpl
{
public:
	Quad quad;
	int targetFrameRate = 60;
	std::vector<ModelObj*> models;

	int currentModelIndex = 0;
	double timer = 0.0f;
};

ObjectQualityScene::ObjectQualityScene()
{
	impl = new ObjectQualitySceneImpl();
}

ObjectQualityScene::~ObjectQualityScene()
{
	if (impl) {
		delete impl;
		impl = nullptr;
	}
}

bool ObjectQualityScene::InitContents()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	for (int i = 1; i < 10; ++i) {
		auto model = new ModelObj();
		model->Load(REDUCED_MODEL_PATH_OF("happybudda", 10.0f * i),
			REDUCED_MODEL_PATH_OF("happybudda", 10.0f * i),
			REDUCED_MODEL_PATH_OF("happybudda", 10.0f * i),
			TARGET_MODEL_BASEPATH);

		model->SetShaders(VS_FILE_PATH, FS_FILE_PATH);

		model->SetPosition(glm::vec3(0, 0, -2.0f));
		model->SetRotation(glm::vec3(0, 0, 0));
		model->SetScale(glm::vec3(0.30f));
		model->SetVisible(true);
		model->SetIsPhysicalObject(false);

		if (!model->Create())
			return false;

		impl->models.push_back(model);
	}

	impl->quad.InitContents();

	return true;
}

void ObjectQualityScene::DestroyContents()
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

int ObjectQualityScene::GetTargetFrameRate()
{
	return impl->targetFrameRate;
}

static bool isStarted = false;

void ObjectQualityScene::OnRender(int cameraIndex, float dt)
{
	if (isStarted) {
		impl->timer += dt;

		if (impl->timer > 3.0f) {
			impl->currentModelIndex = (impl->currentModelIndex + 1) % 10;
			impl->timer = 0.0f;
		}
	}

	static bool isFirstRender = true;

	if (isFirstRender) {
		ML_LOG_TAG(Info, LATENCY, "First Render");
		isFirstRender = false;
	}

	for (int i = 0; i < impl->models.size(); ++i) {
		auto* model = impl->models[i];

		if (i == impl->currentModelIndex) {
			model->SetVisible(true);
		}
		else {
			model->SetVisible(false);
		}

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

int state = 0;

void ObjectQualityScene::OnPressed()
{
	if (state == 0) {
		ML_LOG_TAG(Info, "QUALITY", "Started");

		impl->timer = 0.0f;
		isStarted = true;

		state++;
	} 
	else if (state == 1) {
		ML_LOG_TAG(Info, "QUALITY", "%d", (impl->currentModelIndex + 1) * 10);

		state++;
	}
	else if (state == 2) {
		ML_LOG_TAG(Info, "QUALITY", "RESET");

		state = 0;
	}
}