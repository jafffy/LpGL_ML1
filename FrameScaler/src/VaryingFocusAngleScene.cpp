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

#define FOCUS_MODEL_FILEPATH TARGET_MODEL_BASEPATH "sphere.obj"
#define FOCUS_MODEL_FILEPATH_REDUCED_1 TARGET_MODEL_BASEPATH "sphere_1.obj"
#define FOCUS_MODEL_FILEPATH_REDUCED_2 TARGET_MODEL_BASEPATH "sphere_2.obj"

class VaryingFocusAngleSceneImpl
{
public:
	Quad quad;
	int targetFrameRate = 60;
	std::vector<ModelObj*> models;
	float simplificationAngularFactor = 0.0f;

	float speed = 16.0f;
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

  int n = 64;

  for (int i = 0; i < n; ++i) {
    float t = (float)i / n;
    float c = 5.0f * cosf(t * 2 * M_PI * 100.0f / 180.0f - M_PI * 0.5f);
    float s = 5.0f * sinf(t * 2 * M_PI * 100.0f / 180.0f - M_PI * 0.5f);

    auto model = new ModelObj();
    model->Load(FOCUS_MODEL_FILEPATH,
        FOCUS_MODEL_FILEPATH_REDUCED_1,
        FOCUS_MODEL_FILEPATH_REDUCED_2,
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
  for (int i = 0; i < impl->models.size(); ++i) {
    auto* model = impl->models[i];
    float t = (i % 2 == 0) ? -1 : 1;
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
}
