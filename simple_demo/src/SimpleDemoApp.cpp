#include "SimpleDemoApp.h"
#include "ModelObj.h"
#include "Camera.h"
#include "Experiment.h"
#include "LpGLEngine.h"

#include <functional>
#include <random>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/intersect.hpp"

#if defined(ML1_DEVICE)
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#elif defined(ML1_OSX)

#include <GL/glew.h>

#endif

#include "Quad.h"

struct HotMobile2019DemoEnv {
  static float bunny_scale;
  static int num_bunnies;
  static int num_rows;
};

float HotMobile2019DemoEnv::bunny_scale = 0.5f;
int HotMobile2019DemoEnv::num_bunnies = 15;
int HotMobile2019DemoEnv::num_rows = 5;

class SimpleDemoAppImpl
{
public:
  std::vector<ModelObj *> models = {};
  int targetFrameRate = 60;

  Quad quad;

  bool init_models() {
    float const k_distance_front_of_user = 5;

    int n = HotMobile2019DemoEnv::num_bunnies;
    int n_row = HotMobile2019DemoEnv::num_rows;

    for (int i = 0; i < n; ++i) {
      for (int j = 0; j < n_row; ++j) {
        auto model = new ModelObj();

        model->Load(TARGET_MODEL_FILEPATH,
                    TARGET_MODEL_FILEPATH_REDUCED_1,
                    TARGET_MODEL_FILEPATH_REDUCED_2, TARGET_MODEL_BASEPATH);

        model->SetShaders(VS_FILE_PATH, FS_FILE_PATH);

        model->SetScale(glm::vec3(HotMobile2019DemoEnv::bunny_scale));
        model->SetVisible(true);
        if (!model->Create())
          return false;

        model->SetPosition(
          glm::vec3(-n * 0.5f + i, -n_row * 0.5 + j, -k_distance_front_of_user)); // 5m in front of the user.

        assert(model);

        models.push_back(model);
      }
    }

    return true;
  }
};

SimpleDemoApp::SimpleDemoApp() {
  impl = new SimpleDemoAppImpl();
}

SimpleDemoApp::~SimpleDemoApp()
{
  if (impl) {
    delete impl;
    impl = nullptr;
  }
}

void SimpleDemoApp::Update(float dt)
{
  for (auto *model : impl->models) {
    model->Update(dt);
  }
}

void SimpleDemoApp::OnRender(int cameraIndex, float dt) {
  Update(dt);

  if (cameraIndex == 0) {
    auto& engine = LpGLEngine::instance();

    impl->targetFrameRate = engine.Update(impl->models,
                                          impl->targetFrameRate, dt);
  }

  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  for (auto* model : impl->models)
    model->Render();

  impl->quad.Draw();
}

int SimpleDemoApp::GetTargetFrameRate()
{
  return impl->targetFrameRate;
}

bool SimpleDemoApp::InitContents()
{
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);

  impl->init_models();

  impl->quad.InitContents();

  return true;
}

void SimpleDemoApp::DestroyContents() {
  impl->quad.DestroyContents();

  for (auto* model : impl->models) {
    model->Destroy();

    delete model;
    model = nullptr;
  }

  impl->models.clear();
}

void SimpleDemoApp::OnPressed()
{
  LpGLEngine::instance().IsOn = true;
}

void SimpleDemoApp::OnReleased() {
  LpGLEngine::instance().IsOn = false;
}
