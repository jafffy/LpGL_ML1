#ifndef GRAPHICS_CONTEXT_T_H_
#define GRAPHICS_CONTEXT_T_H_

#include <ml_api.h>

#if defined(ML1_DEVICE)
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <GLES3/gl3.h>
#elif defined(ML1_OSX)

#include <GL/glew.h>

#endif

struct graphics_context_t {
  MLHandle gl_context_handle;

#if defined(ML1_DEVICE)
  EGLDisplay egl_display;
  EGLContext egl_context;
#elif defined(ML1_OSX)
#endif

  MLHandle &gl_context() {
    return gl_context_handle;
  }

  GLuint framebuffer_id{};

  graphics_context_t();

  ~graphics_context_t();

  void makeCurrent();

  void swapBuffers();

  void unmakeCurrent();
};


#endif // GRAPHICS_CONTEXT_T_H_