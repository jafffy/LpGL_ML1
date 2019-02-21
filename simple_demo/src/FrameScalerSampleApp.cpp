#include "FrameScalerSampleApp.h"
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

#include <ml_logging.h>

struct HotMobile2019DemoEnv {
  static bool is_LpGL_on;
  static double toggling_timer;
  static float sphere_scale;
  static int num_spheres;
};

bool HotMobile2019DemoEnv::is_LpGL_on = false;
double HotMobile2019DemoEnv::toggling_timer = 0.0f;
float HotMobile2019DemoEnv::sphere_scale = 0.5f;
int HotMobile2019DemoEnv::num_spheres = 50;

class FrameScalerSampleAppImpl
{
public:
  std::vector<ModelObj *> models = {};
  int targetFrameRate = 60;

  void distributeObjects() {
    float const k_distance_front_of_user = 5;

    for (int i = 0; i < models.size(); ++i) {
      auto *model = models[i];

      model->SetPosition(glm::vec3(i, 0, k_distance_front_of_user));
    }
  }

  bool init_models() {
    int n = HotMobile2019DemoEnv::num_spheres;

    for (int i = 0; i < n; ++i) {
      auto model = new ModelObj();

      model->Load(TARGET_MODEL_FILEPATH,
                  TARGET_MODEL_FILEPATH_REDUCED_1,
                  TARGET_MODEL_FILEPATH_REDUCED_2, TARGET_MODEL_BASEPATH);

      model->SetShaders(VS_FILE_PATH, FS_FILE_PATH);

      model->SetScale(glm::vec3(HotMobile2019DemoEnv::sphere_scale));
      model->SetVisible(true);
      if (!model->Create())
        return false;

      assert(model);
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

void FrameScalerSampleApp::Update(float dt)
{
  for (auto *model : impl->models) {
    model->Update(dt);
  }

  // Demo timer update for toggling
  {
    // FIXME: Make toggle works!
    double& toggling_timer = HotMobile2019DemoEnv::toggling_timer;
    toggling_timer += dt;

    /*
    bool& is_LpGL_on = HotMobile2019DemoEnv::is_LpGL_on;

    if (is_LpGL_on) {
      if (toggling_timer > 20) {
        is_LpGL_on = false;
      }
    }
    else {
      if (toggling_timer > 10) {
        is_LpGL_on = true;
      }
    }
    */
  }
}

void FrameScalerSampleApp::OnRender(int cameraIndex, float dt) {
  static bool isFirstRender = true;

  if (isFirstRender) {
    isFirstRender = false;
  }

  Update(dt);

  if (cameraIndex == 0 // LpGL for one eye.
      && HotMobile2019DemoEnv::is_LpGL_on) {
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

  impl->init_models();

  return true;
}

void FrameScalerSampleApp::DestroyContents()
{
  for (auto* model : impl->models) {
    model->Destroy();

    delete model;
    model = nullptr;
  }

  impl->models.clear();
}

void FrameScalerSampleApp::OnPressed()
{
  bool old_, new_;

  old_ = HotMobile2019DemoEnv::is_LpGL_on;
  new_ = !old_;

  HotMobile2019DemoEnv::is_LpGL_on = new_;

  ML_LOG(Info, "Toggle LpGL: %d -> %d", old_, new_);
}

