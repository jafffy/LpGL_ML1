#include "MLNativeWindow.h"
#include "Camera.h"
#include "Experiment.h"

#if defined(ML1_DEVICE)
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
#elif defined(ML1_OSX)

#include <unistd.h>

#include <GL/glew.h>

#endif

#include <ml_graphics.h>
#include <ml_head_tracking.h>
#include <ml_perception.h>
#include <ml_lifecycle.h>
#include <ml_logging.h>
#include <ml_input.h>

#include <chrono>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/transform.hpp"

#include "graphics_context_t.h"

// Constants
const char application_name[] = "com.jafffy.simple_demo";

// Structures
struct application_context_t {
  int dummy_value;
};


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

class MLNativeWindowImpl
{
public:
  explicit MLNativeWindowImpl(App *app)
		: app(app) {}
	~MLNativeWindowImpl()
	{
		if (app) {
			delete app;
			app = nullptr;
		}
	}

	App* app = nullptr;
	graphics_context_t graphics_context;
	MLHandle graphics_client = ML_INVALID_HANDLE;
};

static MLNativeWindowImpl* g_impl = nullptr;

MLNativeWindow::MLNativeWindow(App* app)
{
	impl = new MLNativeWindowImpl(app);
	g_impl = impl;
}

MLNativeWindow::~MLNativeWindow()
{
	if (impl) {
		delete impl;
		impl = nullptr;
		g_impl = nullptr;
	}
}

static void on_button_down(uint8_t controller_id, MLInputControllerButton button, void *data)
{
	if (!g_impl) {
		return;
	}

	g_impl->app->OnPressed();
}

static void on_button_up(uint8_t controller_id, MLInputControllerButton button, void *data) {
	if (!g_impl) {
		return;
	}

	g_impl->app->OnReleased();
}

int MLNativeWindow::Start()
{ 
	MLLifecycleCallbacks lifecycle_callbacks = {};
  lifecycle_callbacks.on_stop = onStop;
  lifecycle_callbacks.on_pause = onPause;
  lifecycle_callbacks.on_resume = onResume;

	struct application_context_t application_context{};
  application_context.dummy_value = 2;

  if (MLResult_Ok != MLLifecycleInit(&lifecycle_callbacks, (void*)&application_context)) {
    ML_LOG(Error, "%s: Failed to initialize lifecycle.", application_name);
    return -1;
  }

  MLHandle inputHandle;
  MLInputConfiguration inputConfig;
  MLInputCreate(&inputConfig, &inputHandle);

  MLInputControllerCallbacks inputcontroller_callbacks = {};
  inputcontroller_callbacks.on_button_down = on_button_down;
	inputcontroller_callbacks.on_button_up = on_button_up;

  MLInputSetControllerCallbacks(inputHandle, &inputcontroller_callbacks, nullptr);

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
	auto opengl_context = reinterpret_cast<MLHandle>(impl->graphics_context.gl_context());
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

  if (!impl->app->InitContents())
	  return -1;

  ML_LOG(Info, "%s: Start loop.", application_name);

  auto start = std::chrono::steady_clock::now();
  float elapsed_time = 0.0f;

  int FPS = 0;
  float FPStimer = 0.0f;

  while (application_context.dummy_value) {
	  OnRender(elapsed_time / 1000.0f);

	  double target_frame_rate = impl->app->GetTargetFrameRate();
	  auto last_time = std::chrono::steady_clock::now();
	  elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(last_time - start).count();
	  start = last_time;

	  auto expected_time_ms = 1000.0 / target_frame_rate;
	  auto blanked_time_ms = expected_time_ms - elapsed_time;

	  if (blanked_time_ms > 0)
	  {
		  usleep(useconds_t(blanked_time_ms * 1000));
	  }

	  FPStimer += 1 / (elapsed_time / 1000.0f);
	  FPS++;

	  if (FPStimer > 1.0f) {
		  FPStimer = 0.0f;
		  FPS = 0;
	  }
  }

  ML_LOG(Info, "%s: End loop.", application_name);

  impl->app->DestroyContents();

  impl->graphics_context.unmakeCurrent();

  glDeleteFramebuffers(1, &impl->graphics_context.framebuffer_id);

  // clean up system
  MLGraphicsDestroyClient(&impl->graphics_client);
  MLPerceptionShutdown();

  return 0;
}

void MLNativeWindow::OnRender(float dt)
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
		const MLRectf& viewport = virtual_camera_array.viewport;

		glm::vec3 view_pos[2] = {
			glm::make_vec3(virtual_camera_array.virtual_cameras[0].transform.position.values),
			glm::make_vec3(virtual_camera_array.virtual_cameras[1].transform.position.values),
		};
		glm::quat view_rot[2] = {
			glm::make_quat(virtual_camera_array.virtual_cameras[0].transform.rotation.values),
			glm::make_quat(virtual_camera_array.virtual_cameras[1].transform.rotation.values),
		};

		auto mean_view_pos = (view_pos[0] + view_pos[1]) * 0.5f;
		auto mean_view_rot = glm::slerp(view_rot[0], view_rot[1], 0.5f);

		Camera::Instance().P_for_LpGL = glm::perspectiveFov(CULLING_FOV, 1280.0f, 960.0f, 0.1f, 10.0f);
		Camera::Instance().V_for_LpGL = glm::transpose(glm::translate(mean_view_pos) * glm::toMat4(mean_view_rot));;

		glBindFramebuffer(GL_FRAMEBUFFER, impl->graphics_context.framebuffer_id);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, static_cast<GLuint>(virtual_camera_array.color_id),
                              0, camera);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, static_cast<GLuint>(virtual_camera_array.depth_id),
                              0, camera);
		glViewport((GLint)viewport.x, (GLint)viewport.y,
			(GLsizei)viewport.w, (GLsizei)viewport.h);

		auto ml_view_pos = virtual_camera_array.virtual_cameras[camera].transform.position;
		auto ml_view_rot = virtual_camera_array.virtual_cameras[camera].transform.rotation;
		auto ml_P = virtual_camera_array.virtual_cameras[camera].projection;

		auto view_translation = glm::make_vec3(ml_view_pos.values);
		auto view_rotation = glm::make_quat(ml_view_rot.values);

		Camera::Instance().position = view_translation;
		Camera::Instance().rotation = view_rotation;

		Camera::Instance().V = glm::transpose(glm::translate(view_translation) * glm::toMat4(view_rotation));
		Camera::Instance().P = glm::make_mat4(ml_P.matrix_colmajor);
		Camera::Instance().ratio = viewport.w / viewport.h;

		impl->app->OnRender(camera, dt);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		MLGraphicsSignalSyncObjectGL(impl->graphics_client, virtual_camera_array.virtual_cameras[camera].sync_object);
	}
	out_result = MLGraphicsEndFrame(impl->graphics_client, frame_handle);
	if (MLResult_Ok != out_result) {
		ML_LOG(Error, "MLGraphicsEndFrame complained: %d", out_result);
	}

	impl->graphics_context.swapBuffers();
}

