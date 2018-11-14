#include "FrameScalerSampleApp.h"
#include "ModelObj.h"
#include "qds.h"

#include <stdio.h>
#include <stdlib.h>

#include <chrono>
#include <cmath>
#include <vector>

#include <unistd.h>

#ifndef EGL_EGLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#endif

#include <EGL/egl.h>
#include <EGL/eglext.h>

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include <ml_graphics.h>
#include <ml_head_tracking.h>
#include <ml_perception.h>
#include <ml_lifecycle.h>
#include <ml_logging.h>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/transform.hpp"

// Constants
const char application_name[] = "com.magicleap.simpleglapp";

// Structures
struct application_context_t {
  int dummy_value;
};

struct graphics_context_t {
  EGLDisplay egl_display;
  EGLContext egl_context;

  GLuint framebuffer_id;
  GLuint vertex_shader_id;
  GLuint fragment_shader_id;
  GLuint program_id;

  graphics_context_t();
  ~graphics_context_t();

  void makeCurrent();
  void swapBuffers();
  void unmakeCurrent();
};

graphics_context_t::graphics_context_t() {
  egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  EGLint major = 4;
  EGLint minor = 0;
  eglInitialize(egl_display, &major, &minor);
  eglBindAPI(EGL_OPENGL_API);

  EGLint config_attribs[] = {
    EGL_RED_SIZE, 5,
    EGL_GREEN_SIZE, 6,
    EGL_BLUE_SIZE, 5,
    EGL_ALPHA_SIZE, 0,
    EGL_DEPTH_SIZE, 24,
    EGL_STENCIL_SIZE, 8,
    EGL_NONE
  };
  EGLConfig egl_config = nullptr;
  EGLint config_size = 0;
  eglChooseConfig(egl_display, config_attribs, &egl_config, 1, &config_size);

  EGLint context_attribs[] = {
    EGL_CONTEXT_MAJOR_VERSION_KHR, 3,
    EGL_CONTEXT_MINOR_VERSION_KHR, 0,
    EGL_NONE
  };
  egl_context = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, context_attribs);
}

void graphics_context_t::makeCurrent() {
  eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, egl_context);
}

void graphics_context_t::unmakeCurrent() {
  eglMakeCurrent(NULL, EGL_NO_SURFACE, EGL_NO_SURFACE, NULL);
}

void graphics_context_t::swapBuffers() {
  // buffer swapping is implicit on device (MLGraphicsEndFrame)
}

graphics_context_t::~graphics_context_t() {
  eglDestroyContext(egl_display, egl_context);
  eglTerminate(egl_display);
}

// Callbacks
static void onStop(void* application_context)
{
  ((struct application_context_t*)application_context)->dummy_value = 0;
  ML_LOG(Info, "%s: On stop called.", application_name);
}

static void onPause(void* application_context)
{
  ((struct application_context_t*)application_context)->dummy_value = 1;
  ML_LOG(Info, "%s: On pause called.", application_name);
}

static void onResume(void* application_context)
{
  ((struct application_context_t*)application_context)->dummy_value = 2;
  ML_LOG(Info, "%s: On resume called.", application_name);
}

class FrameScalerSampleAppImpl
{
public:
	graphics_context_t graphics_context;
	MLHandle graphics_client = ML_INVALID_HANDLE;
	std::vector<ModelObj*> models;
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
  MLLifecycleCallbacks lifecycle_callbacks = {};
  lifecycle_callbacks.on_stop = onStop;
  lifecycle_callbacks.on_pause = onPause;
  lifecycle_callbacks.on_resume = onResume;

  struct application_context_t application_context;
  application_context.dummy_value = 2;

  if (MLResult_Ok != MLLifecycleInit(&lifecycle_callbacks, (void*)&application_context)) {
    ML_LOG(Error, "%s: Failed to initialize lifecycle.", application_name);
    return -1;
  }

  // initialize perception system
  MLPerceptionSettings perception_settings;
  if (MLResult_Ok != MLPerceptionInitSettings(&perception_settings)) {
    ML_LOG(Error, "%s: Failed to initialize perception.", application_name);
  }

  if (MLResult_Ok != MLPerceptionStartup(&perception_settings)) {
    ML_LOG(Error, "%s: Failed to startup perception.", application_name);
    return -1;
  }

  // Get ready to connect our GL context to the MLSDK graphics API
  impl->graphics_context.makeCurrent();
  glGenFramebuffers(1, &impl->graphics_context.framebuffer_id);

  MLGraphicsOptions graphics_options = { 0, MLSurfaceFormat_RGBA8UNorm, MLSurfaceFormat_D32Float };
  MLHandle opengl_context = reinterpret_cast<MLHandle>(impl->graphics_context.egl_context);
  MLGraphicsCreateClientGL(&graphics_options, opengl_context, &impl->graphics_client);

  // Now that graphics is connected, the app is ready to go
  if (MLResult_Ok != MLLifecycleSetReadyIndication()) {
    ML_LOG(Error, "%s: Failed to indicate lifecycle ready.", application_name);
    return -1;
  }

  MLHandle head_tracker;
  MLResult head_track_result = MLHeadTrackingCreate(&head_tracker);
  MLHeadTrackingStaticData head_static_data;
  if (MLResult_Ok == head_track_result && MLHandleIsValid(head_tracker)) {
    MLHeadTrackingGetStaticData(head_tracker, &head_static_data);
  } else {
    ML_LOG(Error, "%s: Failed to create head tracker.", application_name);
  }

  if (!InitContents())
	  return -1;

  ML_LOG(Info, "%s: Start loop.", application_name);

  while (application_context.dummy_value) {
	  auto start = std::chrono::steady_clock::now();
	  OnRender();
	  qds_update();
	  double target_frame_rate = get_recommended_framerate();
	  auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
	  auto expected_time_ms = 1000.0 / target_frame_rate;
	  auto blanked_time_ms = expected_time_ms - elapsed_time;

	  if (blanked_time_ms > 0)
	  {
		  usleep(useconds_t(blanked_time_ms * 1000));
	  }
  }

  ML_LOG(Info, "%s: End loop.", application_name);

  DestroyContents();

  impl->graphics_context.unmakeCurrent();

  glDeleteFramebuffers(1, &impl->graphics_context.framebuffer_id);

  // clean up system
  MLGraphicsDestroyClient(&impl->graphics_client);
  MLPerceptionShutdown();

  return 0;
}

void FrameScalerSampleApp::Cleanup()
{

}

void FrameScalerSampleApp::Update(float dt)
{

}

void FrameScalerSampleApp::OnRender()
{
	MLGraphicsFrameParams frame_params;

	MLResult out_result = MLGraphicsInitFrameParams(&frame_params);
	if (MLResult_Ok != out_result) {
		ML_LOG(Error, "MLGraphicsInitFrameParams complained: %d", out_result);
	}
	frame_params.surface_scale = 1.0f;
	frame_params.projection_type = MLGraphicsProjectionType_ReversedInfiniteZ;
	frame_params.near_clip = 1.0f;
	frame_params.focus_distance = 1.0f;

	MLHandle frame_handle;
	MLGraphicsVirtualCameraInfoArray virtual_camera_array;
	out_result = MLGraphicsBeginFrame(impl->graphics_client, &frame_params, &frame_handle, &virtual_camera_array);
	if (MLResult_Ok != out_result) {
		ML_LOG(Error, "MLGraphicsBeginFrame complained: %d", out_result);
	}

	for (int camera = 0; camera < virtual_camera_array.num_virtual_cameras; ++camera) {
		glBindFramebuffer(GL_FRAMEBUFFER, impl->graphics_context.framebuffer_id);
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, virtual_camera_array.color_id, 0, camera);
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, virtual_camera_array.depth_id, 0, camera);
		const MLRectf& viewport = virtual_camera_array.viewport;
		glViewport((GLint)viewport.x, (GLint)viewport.y,
			(GLsizei)viewport.w, (GLsizei)viewport.h);

		auto ml_view_pos = virtual_camera_array.virtual_cameras[camera].transform.position;
		auto ml_view_rot = virtual_camera_array.virtual_cameras[camera].transform.rotation;
		auto ml_P = virtual_camera_array.virtual_cameras[camera].projection;

		auto view_translation = glm::make_vec3(ml_view_pos.values);
		auto view_rotation = glm::make_quat(ml_view_rot.values);

		glm::mat4 V = glm::transpose(glm::translate(view_translation) * glm::toMat4(view_rotation));
		glm::mat4 P = glm::make_mat4(ml_P.matrix_colmajor);

		Draw(camera, P * V);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		MLGraphicsSignalSyncObjectGL(impl->graphics_client, virtual_camera_array.virtual_cameras[camera].sync_object);
	}
	out_result = MLGraphicsEndFrame(impl->graphics_client, frame_handle);
	if (MLResult_Ok != out_result) {
		ML_LOG(Error, "MLGraphicsEndFrame complained: %d", out_result);
	}

	impl->graphics_context.swapBuffers();
}

bool FrameScalerSampleApp::InitContents()
{
	int n = 15;

	for (int i = 0; i < n; ++i) {
		double t = i / (double)n;
		double r = 5.0f;
		float s = sinf(t * 2 * M_PI);
		float c = cosf(t * 2 * M_PI);

		auto model = new ModelObj();

		model->Load("assets/models/69K.obj", "assets/models");
		model->SetShaders("assets/shaders/basic3D.vert", "assets/shaders/basic3D.frag");
		model->SetPosition(glm::vec3(r*c, 0, r*s));
		model->SetScale(glm::vec3(5.0f));

		if (!model->Create())
			return false;

		impl->models.push_back(model);
	}

	return true;
}

void FrameScalerSampleApp::DestroyContents()
{
	for (auto* model : impl->models) {
		model->Destroy();

		if (model) {
			delete model;
			model = nullptr;
		}
	}

	impl->models.clear();
}

void FrameScalerSampleApp::Draw(int camera_number, glm::mat4 VP)
{
	for (auto* model : impl->models)
		model->Update(VP);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (auto* model : impl->models)
		model->Render();
}