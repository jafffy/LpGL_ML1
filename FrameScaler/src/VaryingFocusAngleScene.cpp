#include "VaryingFocusAngleScene.h"
#include "Quad.h"
#include "ModelObj.h"
#include "LpGLEngine.h"
#include "Experiment.h"

#include <ml_logging.h>

#include <vector>
#include <cstdlib>

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

class VaryingFocusAngleSceneImpl
{
public:
	Quad quad;
	int targetFrameRate = 60;
	std::vector<ModelObj*> models;
	float simplificationAngularFactor = 0.0f;

	float speed = 16.0f;
};

static bool is_started = true;

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

  int n = 64;

  for (int i = 0; i < n; ++i) {
    float t = (float)i / n;
    float c = 5.0f * cosf(t * 2 * M_PI * 100.0f / 180.0f - M_PI * 0.5f);
    float s = 5.0f * sinf(t * 2 * M_PI * 100.0f / 180.0f - M_PI * 0.5f);

    auto model = new ModelObj();
    model->Load(TARGET_MODEL_FILEPATH,
        TARGET_MODEL_FILEPATH_REDUCED_1,
        TARGET_MODEL_FILEPATH_REDUCED_2,
        TARGET_MODEL_BASEPATH);

    if (i == n / 2)
      model->SetShaders(VS_FILE_PATH, GREEN_FS_FILE_PATH);
    else
      model->SetShaders(VS_FILE_PATH, FS_FILE_PATH);

    model->SetPosition(glm::vec3(c, 0, s));
    model->SetRotation(glm::vec3(0, 0, -t * 2 * M_PI + M_PI * 0.5f + M_PI));
    model->SetScale(glm::vec3(0.25f));
    model->SetVisible(true);
    model->SetInitialVelocity(glm::vec3((float)random() / RAND_MAX - 0.5f, 0, 0));

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

static float timer = 0.0f;

void VaryingFocusAngleScene::OnRender(int cameraIndex, float dt)
{
  if (!is_started)
    timer += dt * impl->speed;

  impl->simplificationAngularFactor += M_PI / 180.0f * dt * 0.3f;

  static bool isFirstRender = true;

  if (isFirstRender) {
    ML_LOG_TAG(Info, LATENCY, "First Render");
    isFirstRender = false;
  }

  for (int i = 0; i < impl->models.size(); ++i) {
    auto* model = impl->models[i];
    float t = (i % 2 == 0) ? -1 : 1;
    // model->SetPosition(glm::vec3(i * 0.25f, 0.5 * sinf(timer + M_PI / 2 * i) * t, -5));
    model->Update(dt);
  }

  if (cameraIndex == 0) {
    LpGLEngine::instance().SetLODSensitivity(impl->simplificationAngularFactor);
    int recommended_fps = LpGLEngine::instance().Update(eels_with_full_lpgl, impl->models, impl->targetFrameRate, dt);
    impl->targetFrameRate = recommended_fps;

    // ML_LOG_TAG(Info, "FRAMERATE", "%f", (float)impl->targetFrameRate);
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
  if (is_started) {
    is_started = false;
    return;
  }

  ML_LOG_TAG(Info, "SIMPLIFY", "Current SIMPLIFY FOV: %f", (LOD_LV2 - impl->simplificationAngularFactor) / M_PI * 180);

  is_started = true;
  impl->simplificationAngularFactor = 0.0f;
}
