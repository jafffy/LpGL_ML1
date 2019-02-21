#ifndef SHADERUTILS_H_
#define SHADERUTILS_H_

#if defined(ML1_DEVICE)
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#elif defined(ML1_OSX)

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#endif

GLuint LoadShaders(const char* verex_file_path, const char* fragment_file_path);

#endif // SHADERUTILS_H_